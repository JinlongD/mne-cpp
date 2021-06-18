//=====================================================================================================================
/**
 * @file     matparser.cpp
 * @author   Christoph Dinh <chdinh@nmr.mgh.harvard.edu>;
 *           Lorenz Esch <lesch@mgh.harvard.edu>;
 *           Viktor Klueber <Viktor.Klueber@tu-ilmenau.de>
 * @since    0.1.0
 * @date     February, 2013
 *
 * @section  LICENSE
 *
 * Copyright (C) 2013, Christoph Dinh, Lorenz Esch, Viktor Klueber. All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without modification, are permitted provided that
 * the following conditions are met:
 *     * Redistributions of source code must retain the above copyright notice, this list of conditions and the
 *       following disclaimer.
 *     * Redistributions in binary form must reproduce the above copyright notice, this list of conditions and
 *       the following disclaimer in the documentation and/or other materials provided with the distribution.
 *     * Neither the name of MNE-CPP authors nor the names of its contributors may be used
 *       to endorse or promote products derived from this software without specific prior written permission.
 *
 * THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS" AND ANY EXPRESS OR IMPLIED
 * WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A
 * PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT OWNER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT,
 * INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO,
 * PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION)
 * HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING
 * NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE
 * POSSIBILITY OF SUCH DAMAGE.
 *
 *
 * @brief    Definition of the matparser class.
 *
 */
//=====================================================================================================================

//=====================================================================================================================
// INCLUDES
//=====================================================================================================================
#include "matparser.h"

//=====================================================================================================================
// QT INCLUDES
//=====================================================================================================================
#include <QSettings>
#include <QDebug>

//=====================================================================================================================
// EIGEN INCLUDES
//=====================================================================================================================

//=====================================================================================================================
// USED NAMESPACES
//=====================================================================================================================
using namespace CLASSIFIERSPLUGIN;

//=====================================================================================================================
// DEFINE MEMBER METHODS
//=====================================================================================================================
MatParser::MatParser(QObject *parent)
    : QThread(parent)
    //, m_qByteArray(byteArray)
    , m_bIsMatFileLoaded(false)
    , m_bIsParsed(false)
    , m_bIsLittleEndian(false)
{
    this->initDataTypeMaps();
}

//=====================================================================================================================
MatParser::~MatParser()
{
    /*m_mutex.lock();
    m_quite = true;
    m_slaveMode = false;
    m_mutex.unlock();*/

    this->quit();
    this->wait();
}

//=====================================================================================================================
void MatParser::initDataTypeMaps()
{
    m_mapSizeOfDataType.insert(miINT8, 1);   // 1 byte
    m_mapSizeOfDataType.insert(miUINT8, 1);  // 1 bytes
    m_mapSizeOfDataType.insert(miINT16, 2);  // 2 bytes
    m_mapSizeOfDataType.insert(miUINT16, 2); // 2 bytes
    m_mapSizeOfDataType.insert(miINT32, 4);  // 4 bytes
    m_mapSizeOfDataType.insert(miUINT32, 4); // 4 bytes
    m_mapSizeOfDataType.insert(miSINGLE, 4); // 4 bytes
    m_mapSizeOfDataType.insert(miDOUBLE, 8); // 8 bytes
    m_mapSizeOfDataType.insert(miINT64, 8);  // 8 bytes
    m_mapSizeOfDataType.insert(miUINT64, 8); // 8 bytes
    m_mapSizeOfDataType.insert(miUTF8, 1);   // 1~4 bytes
    m_mapSizeOfDataType.insert(miUTF16, 2);  // 2 or 4 bytes
    m_mapSizeOfDataType.insert(miUTF32, 4);  // 4 bytes

    m_mapDecoders[miINT8]   = miINT8Decoder;   // 1  // insert does not work here.
    m_mapDecoders[miUINT8]  = miUINT8Decoder;  // 2
    m_mapDecoders[miINT16]  = miINT16Decoder;  // 3
    m_mapDecoders[miUINT16] = miUINT16Decoder; // 4
    m_mapDecoders[miINT32]  = miINT32Decoder;  // 5
    m_mapDecoders[miUINT32] = miUINT32Decoder; // 6
    m_mapDecoders[miSINGLE] = miSINGLEDecoder; // 7
    m_mapDecoders[miDOUBLE] = miDOUBLEDecoder; // 9
    m_mapDecoders[miINT64]  = miINT64Decoder;  // 12
    m_mapDecoders[miUINT64] = miUINT64Decoder; // 13
}

//=====================================================================================================================
void MatParser::setMatFile(const QByteArray &byteArray)
{
    m_qByteArray = byteArray;
    m_bIsParsed = false;
    m_bIsLittleEndian = false;
    m_bIsMatFileLoaded = true;
}

//=====================================================================================================================
bool MatParser::getParsingState()
{
    return m_bIsParsed;
}

//=====================================================================================================================
MatParser::ArrayFlags MatParser::getArrayFlags(QByteArray *byteArray, const bool &bIsLittleEndian)
{
    /** Array Flags Sub-Element (Array Flags subelement is totally 16 bytes, no need to check Small Data Element Format.)
     *  Tag (Tag is fixed format):
     *      Data Type: miUINT32 (sizeOfDataType = 4)
     *      Number of Bytes: 8 = 2*sizeOfDataType;
     *
     *  qint32      iDataTypeSub;       // temp: data types for each subelement.
     *  qint32      iSizeOfDataTypeSub; // temp: size of datatype for each subelement.
     *  qint32      iNumOfBytesSub;     // temp: number of bytes for each subelement.
     *  qint32      iNumOfDataSub;      // temp: number of data for each subelement.
     *  qint32      iNum;               // temp: int.
     */

    byteArray->remove(0, 8); // skip the Tag field of Array Flags Subelement.
    QByteArray byteTemp = byteArray->left(8); // get the Data of Array Flags Subelement.
    byteArray->remove(0, 8);

    ArrayFlags arrayFlags;
    QByteArray temp;
    if (bIsLittleEndian) {
        temp = byteTemp.left(4);
        std::reverse(temp.begin(), temp.end()); // Little Endian format, byte-swapping is needed.
        QByteArray ba = byteTemp.right(4);
        std::reverse(ba.begin(), ba.end());
        temp.insert(4, ba);
    } else {
        temp = byteTemp;   // Big Endian format, no byte-swapping.
    }

    qint8 iFlags = temp.mid(2, 1).toHex().toInt(nullptr, 16);      // Flags
    if ((iFlags & 0x08) == 0x08) {
        // check the complex flag
        arrayFlags.bIsComplex = true;
    }

    if ((iFlags & 0x04) == 0x04) {
        // check the global flag
        arrayFlags.bIsGlobal = true;
    }

    if ((iFlags & 0x02) == 0x02) {
        // check the logical flag
        arrayFlags.bIsLogical = true;
    }

    arrayFlags.iClass = temp.mid(3, 1).toHex().toInt(nullptr, 16); // Classes
    if (arrayFlags.iClass == mxSPARSE_CLASS) {
        arrayFlags.iNzMax = temp.right(4).toHex().toInt(nullptr, 16);
    }

    return arrayFlags;
}

//=====================================================================================================================
QList<qint32> MatParser:: getArrayDimensions(QByteArray *byteArray, const bool &bIsLittleEndian)
{
    /** Dimensions Array Sub-Element (miMATRIX format has at least 2 dimensions, no need to check Small Data Element Format.)
     *  Tag:
     *      Data Type: miINT32 (sizeOfDataType = 4)
     *      Number of Bytes: numberOfDimensions * sizeOfDataType
     *
     *  Dimensions are decoded and stored as a list of qint32 numbers.
     *  Example: a 2-by-3-by-4 MATLAB array (3D matrix), this functions returns a qlist<qin32> list = {2, 3, 4}.
     *
     *  qint32      iDataTypeSub;       // temp: data types for each subelement.
     *  qint32      iSizeOfDataTypeSub; // temp: size of datatype for each subelement.
     *  qint32      iNumOfBytesSub;     // temp: number of bytes for each subelement.
     *  qint32      iNumOfDataSub;      // temp: number of data for each subelement.
     */

    byteArray->remove(0, 4); // skip the Tag: Data type of Dimensions Array Subelement.
    QByteArray byteTemp = byteArray->left(4); // get Number Of Bytes from the Tag field of Dimensions Array Subelement.
    byteArray->remove(0, 4);
    if (bIsLittleEndian) {
        std::reverse(byteTemp.begin(), byteTemp.end());
    }
    qint32 iNumOfBytesSub = byteTemp.toHex().toInt(nullptr, 16);
    qint32 iNumOfDataSub = iNumOfBytesSub/4; // iSizeOfDataTypeSub = sizeOf(miINT32) = 4;

    // Data (dimensions)
    QList<qint32> iDimensions;
    QString str = "[";
    while (iDimensions.size() < iNumOfDataSub) {
        byteTemp = byteArray->left(8); //iSizeOfDataTypeSub = 4, but here consideres padding
        byteArray->remove(0, 8);
        if (bIsLittleEndian) {
            std::reverse(byteTemp.begin(), byteTemp.end());
        }
        qint32 iNum = byteTemp.right(4).toHex().toInt(nullptr, 16);
        if (iNum != 0) {
            iDimensions.append(iNum);
            str.append(QString("%1, ").arg(iNum));
        }
        iNum = byteTemp.left(4).toHex().toInt(nullptr, 16);
        if (iNum != 0) {
            iDimensions.append(iNum);
            str.append(QString("%1, ").arg(iNum));
        }
    }
    str.remove(str.size()-2, 2);
    str.append("]");
    m_sDispDimensions = str;
    return iDimensions;
}

//=====================================================================================================================
QString MatParser::getArrayName(QByteArray *byteArray, const bool &bIsLittleEndian)
{
    /** Array Name Sub-Element (check Small Data Element Format.)
     *  Tag:
     *      Data Type: miINT8 (sizeOfDataType (mapSizeOfDataType[iDataTypeSub]) = 1)
     *      Number of Bytes: numberOfCharacters * sizeOfDataType
     *
     *  qint32      iDataTypeSub;       // temp: data types for each subelement.
     *  qint32      iSizeOfDataTypeSub; // temp: size of datatype for each subelement.
     *  qint32      iNumOfBytesSub;     // temp: number of bytes for each subelement.
     *  qint32      iNumOfDataSub;      // temp: number of data for each subelement.
     */

    QByteArray byteTemp = byteArray->left(4);  // Tag:: check small data element format
    byteArray->remove(0, 4);
    if (bIsLittleEndian) {
        std::reverse(byteTemp.begin(), byteTemp.end());
    }
    qint32 iNum = byteTemp.left(2).toHex().toInt(nullptr, 16);
    QString arrayName;
    qint32 iNumOfBytesSub;

    if ( iNum == 0) {
        // normal subelement
        byteTemp = byteArray->left(4);
        byteArray->remove(0, 4);
        if (bIsLittleEndian) {
            std::reverse(byteTemp.begin(), byteTemp.end());
        }
        iNumOfBytesSub = byteTemp.toHex().toInt(nullptr, 16); // 2*sizeOfDataType

        // Data (Characters: array name)
        while (arrayName.size() < iNumOfBytesSub) {
            byteTemp = byteArray->left(8); // note the zero-padding.
            byteArray->remove(0, 8);
            // byte-swaping is not needed here.
            arrayName += QString::fromLocal8Bit(byteTemp.constData());
        }
    } else {
        // small data element format
        iNumOfBytesSub = iNum; // 1*sizeOfDataType // number of characters, since iSizeOfDataTypeSub = 1;

        // Data (Characters: array name)
        byteTemp = byteArray->left(4); // note the zero-padding for small data element format.
        byteArray->remove(0, 4);
        // byte-swaping is not needed here.
        arrayName = QString::fromLocal8Bit(byteTemp.constData()); // also works: arrayName.append(byteTemp.constData());
    }

    return arrayName;
}

//=====================================================================================================================
QString MatParser::getCharacterArrayData(QByteArray *byteArray, const bool &bIsLittleEndian)
{
    /** Real Part Sub-Element (check Small Data Element Format.) for mxCHAR_CLASS.
     *  Unicode encoded character data (UTF-16, UTF-32) are in the byte order specified by the Endian Indicator.
     *  UFT-8 is byte order neutral (no byte-swapping is needed).
     *  Character data that are not Unicode encoded, the Data Type of the Tag field should be miUNIT16.
     *
     *  Tag:
     *      Data Type: data type should be miUINT16 (4) for mxCHAR_CLASS.
     *      Number of Bytes: numberOfCharacters * sizeOfDataType
     *
     *  Character Arrays are decoded and stored as a QString.
     *  Example: a 3-by-1 Matlab char array ['a'; 'b'; 'c'], this function returns a qstring "abc"
     *          But note that, Dimensions for this array is still a list {3, 1}.
     *
     *  qint32      iDataTypeSub;       // temp: data types for each subelement.
     *  qint32      iSizeOfDataTypeSub; // temp: size of datatype for each subelement.
     *  qint32      iNumOfBytesSub;     // temp: number of bytes for each subelement.
     *  qint32      iNumOfDataSub;      // temp: number of data for each subelement.
     */

    qint32 iDataType;
    qint32 iSizeOfDataType;
    qint32 iNumOfBytes;
    qint32 iNumOfData;

    QString strData;

    QByteArray byteTemp = byteArray->left(4); // chech small data element format
    byteArray->remove(0, 4);
    if (bIsLittleEndian) {
        std::reverse(byteTemp.begin(), byteTemp.end());
    }
    qint32 iNum = byteTemp.left(2).toHex().toInt(nullptr, 16);

    if ( iNum == 0) {
        // normal subelement
        iDataType = byteTemp.toHex().toInt(nullptr, 16);    // get data type
        iSizeOfDataType = m_mapSizeOfDataType[iDataType];   // get size of data type

        byteTemp = byteArray->left(4);
        byteArray->remove(0, 4);
        if (bIsLittleEndian) {
            std::reverse(byteTemp.begin(), byteTemp.end());
        }
        iNumOfBytes = byteTemp.toHex().toInt(nullptr, 16);  // get number of bytes: numberOfCharacters * sizeOfDataType
        iNumOfData = iNumOfBytes/iSizeOfDataType;           // get number of data (numberOfCharacters): number of data values;

        // Data (Characters array, decoded as QString here.)
        for (int i = 0; i < iNumOfData; ++i) {
            byteTemp = byteArray->left(iSizeOfDataType);
            byteArray->remove(0, iSizeOfDataType);
            // byte-swaping is not needed here.
            strData += QString::fromLocal8Bit(byteTemp.constData());
        }

        qint32 remainder = iNumOfBytes % 8;
        if (remainder > 0) {
            byteArray->remove(0, 8 - remainder);    // update the position index for zero-padding
        }
    } else {
        // small data element format
        iNumOfBytes = iNum; // get number of bytes
        iDataType = byteTemp.right(2).toHex().toInt(nullptr, 16);   // get data type: miUINT16 (4)
        iSizeOfDataType = m_mapSizeOfDataType[iDataType];           // 2 for miUINT16
        iNumOfData = iNumOfBytes/iSizeOfDataType;                   // number of data values;

        // Data (characters array, decoded as QString here.)
        for (int i = 0; i < iNumOfData; ++i) {
            byteTemp = byteArray->left(iSizeOfDataType);
            byteArray->remove(0, iSizeOfDataType);
            // byte-swaping is not needed here.
            strData += QString::fromLocal8Bit(byteTemp.constData());
        }

        if (iNumOfBytes < 4) {
            byteArray->remove(0, 4 - iNumOfBytes);  // update the position index for zero-padding
        }
    }

    return strData;
}

//=====================================================================================================================
QList<Eigen::MatrixXd> MatParser::getNumericArrayData(QByteArray *byteArray, const bool &bIsLittleEndian, const QList<qint32> &iDimensions)
{
    /** Real/Imaginary Part Sub-Element (check Small Data Element Format.) for numeric arrays, including:
     *  mxDOUBLE_CLASS, mxSINGLE_CLASS, mxINT8_CLASS, mxUINT8_CLASS, mxINT16_CLASS, mxUINT16_CLASS, mxINT32_CLASS, mxUINT32_CLASS.
     *
     *  Tag:
     *      Data Type: Any of the numeric data types.
     *      Number of Bytes: numberOfVales * sizeOfDataType
     *
     *  High-dimension Matrix (3D-matrix or higher) are decoded as a list of 2D matrices.
     *  Example: A is a 2-by-3-by-4-by-5 MATLAB array (4D matrix, with Dimensions as a list {2, 3, 4, 5}),
     *          this function returns a list of 20 Eigen matrices with dimension 2-by-3, ordered as
     *          A(:, :, 1, 1), A(:, :, 2, 1), A(:, :, 3, 1), A(:, :, 4, 1),
     *          A(:, :, 1, 2), A(:, :, 2, 2), A(:, :, 3, 2), A(:, :, 4, 2),
     *          A(:, :, 1, 3), A(:, :, 2, 3), A(:, :, 3, 3), A(:, :, 4, 3),
     *          A(:, :, 1, 4), A(:, :, 2, 4), A(:, :, 3, 4), A(:, :, 4, 4),
     *          A(:, :, 1, 5), A(:, :, 2, 5), A(:, :, 3, 5), A(:, :, 4, 5).
     *
     *  qint32      iDataTypeSub;       // temp: data types for each subelement.
     *  qint32      iSizeOfDataTypeSub; // temp: size of datatype for each subelement.
     *  qint32      iNumOfBytesSub;     // temp: number of bytes for each subelement.
     *  qint32      iNumOfDataSub;      // temp: number of data for each subelement.
     */

    QList<Eigen::MatrixXd> matData; // all numeric data are decoded and then stored as double in Eigen matrice.
    Eigen::MatrixXd matBase(iDimensions.at(0), iDimensions.at(1));
    qint64 iBaseSize = iDimensions.at(0) * iDimensions.at(1);

    qint32 iDataType;
    qint32 iSizeOfDataType;
    qint32 iNumOfBytes;
    qint32 iNumOfData;

    QByteArray byteTemp = byteArray->left(4); // chech small data element format
    byteArray->remove(0, 4);
    if (bIsLittleEndian) {
        std::reverse(byteTemp.begin(), byteTemp.end());
    }
    qint32 iNum = byteTemp.left(2).toHex().toInt(nullptr, 16);
    if ( iNum == 0) {
        // normal subelement
        iDataType = byteTemp.toHex().toInt(nullptr, 16);    // get Data Type from Tag field.
        iSizeOfDataType = m_mapSizeOfDataType[iDataType];   // get size of Data Type.

        byteTemp = byteArray->left(4);
        byteArray->remove(0, 4);
        if (bIsLittleEndian) {
            std::reverse(byteTemp.begin(), byteTemp.end());
        }
        iNumOfBytes = byteTemp.toHex().toInt(nullptr, 16);  // get Number of Bytes from Tag field: numberOfValues * sizeOfDataType
        iNumOfData = iNumOfBytes/iSizeOfDataType;           // compute number of data values (numberOfValues);

        // Data (numeric array, using cast for decoding numeric array)
        qint32 iNumBase = iNumOfData/iBaseSize;
        pDECODERS pDecoder = m_mapDecoders[iDataType];
        for (int i = 0; i < iNumBase; ++i) {
            for (int icol = 0; icol < iDimensions.at(1); ++icol) {
                for (int irow = 0; irow < iDimensions.at(0); ++irow) {
                    byteTemp = byteArray->left(iSizeOfDataType);
                    byteArray->remove(0, iSizeOfDataType);
                    // byte-swaping is not needed here.
                    matBase(irow, icol) = pDecoder(byteTemp); // using cast to decode numeric data.
                }
            }
            matData.append(matBase);
        }

        qint32 remainder = iNumOfBytes % 8;
        if (remainder > 0) {
            byteArray->remove(0, 8 - remainder); // update the position index for zero-padding
        }
    } else {
        // small data element format
        iNumOfBytes = iNum;                                         // get Number of Bytes from Tag field.
        iDataType = byteTemp.right(2).toHex().toInt(nullptr, 16);   // get Data Type from Tag field.
        iSizeOfDataType = m_mapSizeOfDataType[iDataType];           // get size of Data Type.
        iNumOfData = iNumOfBytes/iSizeOfDataType;                   // compute number of data values (numberOfValues);

        // Data (numeric array, using cast for decoding numeric array)
        qint32 iNumBase = iNumOfData/iBaseSize;
        pDECODERS pDecoder = m_mapDecoders[iDataType];
        for (int i = 0; i < iNumBase; ++i) {
            for (int icol = 0; icol < iDimensions.at(1); ++icol) {
                for (int irow = 0; irow < iDimensions.at(0); ++irow) {
                    byteTemp = byteArray->left(iSizeOfDataType);
                    byteArray->remove(0, iSizeOfDataType);
                    // byte-swaping is not needed here.
                    //matBase(irow, icol) = *(double*)(byteTemp.constData()); // using cast to decode double numbers.
                    matBase(irow, icol) = pDecoder(byteTemp);
                }
            }
            matData.append(matBase);
        }

        if (iNumOfBytes < 4) {
            byteArray->remove(0, 4 - iNumOfBytes); // update the position index for zero-padding
        }
    }
    return matData;
}


//=====================================================================================================================
void MatParser::run()
{
    /** Only compressed/uncompressed miMATRIX (MATLAB arrays) format is supported.
     *
     *  A MAT file is structured as:
     *  Header, Data Element1, Data Element2, ..., repeat tagged data elements until end-of-file.
     *
     *  Each Data Element is a MATLAB variable, which is compressed individually.
     */

    QByteArray  byteArray = m_qByteArray; // make a copy of the original binary MAT-File.
    QByteArray  byteMiMatrix; // byteArray for each variable.
    QByteArray  byteTemp;

    qint32      iDataType;          // data types for the Data Element of the mat-file.
    qint32      iNumOfBytes;        // number of bytes for the Data Element of the mat-file.
    bool        ok = false;

    byteTemp = byteArray.left(128); // header (128 bytes: 124 bytes header text, 2 bytes Version, 2 bytes Endian Indicator
    byteArray.remove(0, 128);
    if (byteTemp.right(2) != ENDIAN_INDICATOR) {
        m_bIsLittleEndian = true;
    }

    // one matlab variable is parsed during each while-loop
    byteTemp = byteArray.left(4); // Data Type from the Tag field of Data Element
    byteArray.remove(0, 4);
    if (m_bIsLittleEndian) {
        std::reverse(byteTemp.begin(), byteTemp.end());
    }
    iDataType = byteTemp.toHex().toInt(&ok, 16);

    if (iDataType == miCOMPRESSED) {
        byteTemp = byteArray.left(4); // NumberOfBytes from the Tag field of Data Element
        //byteArray.remove(0, imaxSize); // remove later, NumberOfBytes is necessary for qUncompress.
        if (m_bIsLittleEndian) {
            std::reverse(byteTemp.begin(), byteTemp.end());
        }
        iNumOfBytes = byteTemp.toHex().toInt(&ok, 16);

        byteMiMatrix = qUncompress(byteArray.left(iNumOfBytes+4)); // using NumberOfBytes and data subelement for uncompression.
        byteArray.remove(0, iNumOfBytes+4);

        byteTemp = byteMiMatrix.left(4); // Data Type from the Tag field of the uncompressed Data Element
        byteMiMatrix.remove(0, 4);
        if (m_bIsLittleEndian) {
            std::reverse(byteTemp.begin(), byteTemp.end());
        }
        iDataType = byteTemp.toHex().toInt(&ok, 16);

        if (iDataType != miMATRIX) {
            emit sig_updateVariableInfo(tr("!Warning: Unsupported data type: %1. Parsing stopped (parsingMAT).").arg(iDataType));
            return;
        }
        byteTemp = byteMiMatrix.left(4); // Number of Bytes from the Tag field of the uncompressed Data Element
        byteMiMatrix.remove(0, 4);
        if (m_bIsLittleEndian) {
            std::reverse(byteTemp.begin(), byteTemp.end());
        }
        iNumOfBytes = byteTemp.toHex().toInt(&ok, 16);
    } else if (iDataType == miMATRIX) {
        byteTemp = byteArray.left(4);
        byteArray.remove(0, 4);
        if (m_bIsLittleEndian) {
            std::reverse(byteTemp.begin(), byteTemp.end());
        }
        iNumOfBytes = byteTemp.toHex().toInt(&ok, 16);

        byteMiMatrix = byteArray.left(iNumOfBytes); // miMATRIX Data
        byteArray.remove(0, iNumOfBytes);
    } else {
        emit sig_updateVariableInfo(tr("!Warning: Unsupported data type: %1. Parsing stopped (parsingMAT).").arg(iDataType));
        return;
    }

    // Dealing with miMATRIX Format Data Elements.
    ArrayFlags aFlags = getArrayFlags(&byteMiMatrix, m_bIsLittleEndian);                // Array Flags Subelement
    QList<qint32> aDimensions = getArrayDimensions(&byteMiMatrix, m_bIsLittleEndian);   // Dimensions Array Subelement
    QString aName = getArrayName(&byteMiMatrix, m_bIsLittleEndian);                     // Array Name Subelement

    if ((aFlags.iClass >= mxDOUBLE_CLASS) && (aFlags.iClass <= mxUINT64_CLASS)) {
        // Numeric Array (6~15)
        DataNumericArray strDataNumeric;
        strDataNumeric.sName = aName;
        strDataNumeric.bIsComplex = aFlags.bIsComplex;
        strDataNumeric.bIsGlobal = aFlags.bIsGlobal;
        strDataNumeric.bIsLogical = aFlags.bIsLogical;
        strDataNumeric.iDimensions = aDimensions;
        strDataNumeric.matPR = getNumericArrayData(&byteMiMatrix, m_bIsLittleEndian, aDimensions); // get real part (pr)

        if (aFlags.bIsComplex) {
            QList<Eigen::MatrixXd> m_dataPI;
            strDataNumeric.matPI = getNumericArrayData(&byteMiMatrix, m_bIsLittleEndian, aDimensions); // get imaginary part (pi)
        }
        emit sig_updateVariableInfo(tr("Array Name: %1 (Numeric Array, Dimensions: %2, Complex: %3) (1: true; 0: false).")
                                    .arg(aName)
                                    .arg(m_sDispDimensions)
                                    .arg(aFlags.bIsComplex));
    } else if (aFlags.iClass == mxCELL_CLASS) {
        // Cell Array (1)
    } else if (aFlags.iClass == mxSTRUCT_CLASS) {
        // Structure (2)
        qDebug() << "struct flags:" << aFlags.bIsComplex << aFlags.bIsGlobal << aFlags.bIsLogical << aFlags.iClass << aFlags.iNzMax;
        qDebug() << "variable name:" << aName << "dimensions:" << aDimensions;

        // basically Field Name Length subelement should always be in small data format.
        byteTemp = byteMiMatrix.left(4);
        byteMiMatrix.remove(0, 4);
        if (m_bIsLittleEndian) {
            std::reverse(byteTemp.begin(), byteTemp.end());
        }
        qint32 iNum = byteTemp.left(2).toHex().toInt(&ok, 16);
        iDataType = byteTemp.right(2).toHex().toInt(&ok, 16);
        qint32 iFieldNameLen;
        if (iNum != 0) {
            // small data format
            iNumOfBytes = iNum;
            byteTemp = byteMiMatrix.left(4);
            byteMiMatrix.remove(0, 4);
            if (m_bIsLittleEndian) {
                std::reverse(byteTemp.begin(), byteTemp.end());
            }
            iFieldNameLen = byteTemp.toHex().toInt(&ok, 16);
            qDebug() << "small data format element";
        } else {
            // normal
            byteTemp = byteMiMatrix.left(4);
            byteMiMatrix.remove(0, 4);
            if (m_bIsLittleEndian) {
                std::reverse(byteTemp.begin(), byteTemp.end());
            }
            iNumOfBytes = byteTemp.toHex().toInt(&ok, 16);

            byteTemp = byteMiMatrix.left(iNumOfBytes);
            byteMiMatrix.remove(0, iNumOfBytes);
            if (m_bIsLittleEndian) {
                std::reverse(byteTemp.begin(), byteTemp.end());
            }
            iFieldNameLen = byteTemp.toHex().toInt(&ok, 16); // Maximum length of field names is 32 (31 characters and a NULL terminator;
        }

        qDebug() << "data type:" << iDataType << "number of bytes:" << iNumOfBytes << "field name length:" << iFieldNameLen;

        // field name subelement
        byteTemp = byteMiMatrix.left(4);
        byteMiMatrix.remove(0, 4);
        if (m_bIsLittleEndian) {
            std::reverse(byteTemp.begin(), byteTemp.end());
        }
        iDataType = byteTemp.toHex().toInt(&ok, 16);

        byteTemp = byteMiMatrix.left(4);
        byteMiMatrix.remove(0, 4);
        if (m_bIsLittleEndian) {
            std::reverse(byteTemp.begin(), byteTemp.end());
        }
        iNumOfBytes = byteTemp.toHex().toInt(&ok, 16);
        qint32 iNumOfFields = iNumOfBytes/iFieldNameLen;
        qDebug() << "field names subelement: data type:" << iDataType << "number of bytes:" << iNumOfBytes
                 << "number of fields:" << iNumOfFields;

        for (int i = 0; i < iNumOfFields; ++i) {
            byteTemp = byteMiMatrix.left(iFieldNameLen); // get field names
            byteMiMatrix.remove(0, iFieldNameLen);
            if (m_bIsLittleEndian) {
                //std::reverse(byteTemp.begin(), byteTemp.end());
            }
            qDebug() << QString::fromLocal8Bit(byteTemp.constData()) << "\n";
        }


        // fields data
        /*for (int i = 0; i < iNumOfFields; ++i) {
                imaxSize = 4; // Tag: data type
                byteTemp = byteMiMatrix.left(imaxSize);
                byteMiMatrix.remove(0, imaxSize);
                if (m_bIsLittleEndian) {
                    std::reverse(byteTemp.begin(), byteTemp.end());
                }
                iDataType = byteTemp.toHex().toInt(&ok, 16);
                imaxSize = 4; // Number of Bytes from the Tag field of Data Element
                byteTemp = byteMiMatrix.left(imaxSize);
                byteMiMatrix.remove(0, imaxSize);
                if (m_bIsLittleEndian) {
                    std::reverse(byteTemp.begin(), byteTemp.end());
                }
                iNumOfBytes = byteTemp.toHex().toInt(&ok, 16);
                qDebug() << "field subelement: data type:" << iDataType << "number of bytes:" << iNumOfBytes;

                aFlags = getArrayFlags(&byteMiMatrix, m_bIsLittleEndian);                // Array Flags Subelement
                aDimensions = getArrayDimensions(&byteMiMatrix, m_bIsLittleEndian);   // Dimensions Array Subelement
                aName = getArrayName(&byteMiMatrix, m_bIsLittleEndian);                     // Array Name Subelement
                qDebug() << "struct flags:" << aFlags.bIsComplex << aFlags.bIsGlobal << aFlags.bIsLogical << aFlags.iClass << aFlags.iNzMax;
                qDebug() << "variable name:" << aName << "dimensions:" << aDimensions;

                QList<Eigen::MatrixXd> dataPR = getNumericArrayData(&byteMiMatrix, m_bIsLittleEndian, aDimensions);
                for (int ii = 0; ii < dataPR.size(); ++ii) {
                    std::cout << dataPR.at(ii) << std::endl;
                }

                if (aFlags.bIsComplex) {
                    QList<Eigen::MatrixXd> dataPI = getNumericArrayData(&byteMiMatrix, m_bIsLittleEndian, aDimensions);
                    for (int ii = 0; ii < dataPR.size(); ++ii) {
                        std::cout << dataPI.at(ii) << std::endl;
                    }
                }
            }*/

    } else if (aFlags.iClass == mxOBJECT_CLASS) {
        // Object (3)
    } else if (aFlags.iClass == mxCHAR_CLASS) {
        // Character Array 4
    } else if (aFlags.iClass == mxSPARSE_CLASS) {
        // Sparse Array (5)
    } else {
        emit sig_updateVariableInfo(tr("!Warning: Invalid array types (CLASS flag: %1). Parsing stopped (parsingMAT).")
                                    .arg(aFlags.iClass));
        return;
    }
}
