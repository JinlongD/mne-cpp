//=====================================================================================================================
/**
 * @file     matparser.h
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
 * @brief    Contains the declaration of the matparser class.
 *
 */
//=====================================================================================================================
#ifndef MATPARSER_H
#define MATPARSER_H

//=====================================================================================================================
// INCLUDES
//=====================================================================================================================
#include "classifiers_global.h"

//=====================================================================================================================
// QT INCLUDES
//=====================================================================================================================
#include <QThread>
#include <QMap>

//=====================================================================================================================
// EIGEN INCLUDES
//=====================================================================================================================
#include <Eigen/Core>

//=====================================================================================================================
// FORWARD DECLARATIONS
//=====================================================================================================================

//=====================================================================================================================
// DEFINE NAMESPACE CLASSIFIERSPLUGIN
//=====================================================================================================================
namespace CLASSIFIERSPLUGIN
{

//=====================================================================================================================
// CLASSIFIERSPLUGIN FORWARD DECLARATIONS
//=====================================================================================================================

//=====================================================================================================================
/**
 * DECLARE CLASS MatParser
 *
 * @brief The MatParser class provides functionalities to parse MATLAB MAT-Format files.
 */
class MatParser : public QThread
{
    Q_OBJECT
public:
    //=====================================================================================================================
    /**
     * @brief MatParser     constructs a MatParser.
     * @param byteArray     original binary mat-file read in as ByteArray.
     * @param parent        parent.
     */
    //explicit MatParser(const QByteArray &byteArray = 0, QObject *parent = nullptr);
    explicit MatParser(QObject *parent = nullptr);

    //=====================================================================================================================
    /**
     * @brief ~MatParser    destructs the MatParser.
     */
    ~MatParser();

    //=====================================================================================================================
    /**
     * @brief setMatFile        set the original MAT-File, which results in refreshing everything.
     * @param byteArray         the ByteArray for the binary mat-file.
     */
    void setMatFile(const QByteArray &byteArray);

    //=====================================================================================================================
    /**
     * @brief getParsingState   get the parsing state
     * @return bool             return true if the mat-file has already parsed.
     */
    bool getParsingState();

protected:
    /**
     * @brief run   do the assigned works in this thread.
     */
    void run() override;

private:
    //=====================================================================================================================
    /**
     * @brief The ArrayFlags struct
     */
    struct ArrayFlags {
        bool    bIsComplex = false;
        bool    bIsGlobal = false;
        bool    bIsLogical = false;
        qint8   iClass = -1;
        qint32  iNzMax = -1;
    };

    //=====================================================================================================================
    /**
     * @brief The NumericArray Data struct
     */
    struct DataNumericArray {
        QString         sName;
        bool            bIsComplex = false;
        bool            bIsGlobal = false;
        bool            bIsLogical = false;
        QList<qint32>   iDimensions;
        QList<Eigen::MatrixXd> matPR;  // real part
        QList<Eigen::MatrixXd> matPI;  // imaginary part

    };

    //=====================================================================================================================
    /**
     * @brief initDataTypeMaps
     */
    void initDataTypeMaps();

    //=====================================================================================================================
    /**
     * @brief getArrayFlags     parsing the Array Flags Subelement.
     * @param byteArray         QByteArray of Array Flags Subelement.
     * @param bIsLittleEndian   Endian indicator, true if the binary file is in Little Endian.
     * @return ArryFlags        structured data of Array Flags Subelement.
     */
    ArrayFlags getArrayFlags(QByteArray *byteArray, const bool &bIsLittleEndian);

    //=====================================================================================================================
    /**
     * @brief getArrayDimensions    parsing the Dimensions Array Subelement.
     * @param byteArray             QByteArray of Dimensions Array Subelement.
     * @param bIsLittleEndian       Endian indicator, true if the binary file is in Little Endian.
     * @return dimensions           a list of dimensions of the miMATRIX subelement.
     */
    QList<qint32> getArrayDimensions(QByteArray *byteArray, const bool &bIsLittleEndian);

    //=====================================================================================================================
    /**
     * @brief getArrayName          parsing the Array Name Subelement.
     * @param byteArray             QByteArray of Array Name Subelement.
     * @param bIsLittleEndian       Endian indicator, true if the binary file is in Little Endian.
     * @return array name           name of the current miMATRIX subelement.
     */
    QString getArrayName(QByteArray *byteArray, const bool &bIsLittleEndian);

    //=====================================================================================================================
    /**
     * @brief getCharacterArrayData parsing the Character Array (Data) Subelement.
     * @param byteArray             QByteArray of Character Array Subelement.
     * @param bIsLittleEndian       Endian indicator, true if the binary file is in Little Endian.
     * @return QString              save the character array as a string.
     */
    QString getCharacterArrayData(QByteArray *byteArray, const bool &bIsLittleEndian);

    //=====================================================================================================================
    /**
     * @brief getNumericArrayData   parsing the Numeric Array (Real/Imaginary Part) Subelement.
     * @param byteArray             QByteArray of Character Array Subelement.
     * @param bIsLittleEndian       Endian indicator, true if the binary file is in Little Endian.
     * @param iDimensions           dimensions of the current subelement.
     * @return list of matrices     a list of Eigen matrices.
     */
    QList<Eigen::MatrixXd> getNumericArrayData(QByteArray *byteArray, const bool &bIsLittleEndian, const QList<qint32> &iDimensions);

    //=====================================================================================================================
    QByteArray              m_qByteArray;
    bool                    m_bIsMatFileLoaded;
    bool                    m_bIsParsed;
    bool                    m_bIsLittleEndian;  // Endian Indicator, if true, byte-swaping is required.
    QString                 m_sDispDimensions;          // for display...

    typedef double (*pDECODERS) (const QByteArray &byteArray);  // define type: pointer to decoder functions (allocate one when needed)
    //double (*m_pDecoders) (const QByteArray &byteArray);        // or can directly declare a pointer to decoder functions.
    std::map<qint8, pDECODERS> m_mapDecoders;                   // using map (vector also works) to link DataType with Decoders.

    // mat-data
    QList<QByteArray>           m_lVariables;
    QStringList                 m_lVarNames;
    QStringList                 m_lVarTypes;

    QStringList                 m_lFieldNames;
    QStringList                 m_lFieldTypes;
    QStringList                 m_l

    //=====================================================================================================================
    // constants
    const qint16 MAT_VERSION       = 0x0100;    // Header flag field: Version
    const QByteArray ENDIAN_INDICATOR = "MI";   // big-endian; If "IM", little-endian is used and byte-swaping is required.

    // Tag field: MAT-File Data Types
    const qint8 miINT8            = 0x01;     // 1  : 8-bit, signed
    const qint8 miUINT8           = 0x02;     // 2  : 8-bit, unsigned
    const qint8 miINT16           = 0x03;     // 3  : 16-bit, signed
    const qint8 miUINT16          = 0x04;     // 4  : 16-bit, unsigned
    const qint8 miINT32           = 0x05;     // 5  : 32-bit, signed
    const qint8 miUINT32          = 0x06;     // 6  : 32-bit, unsigned
    const qint8 miSINGLE          = 0x07;     // 7  : 32-bit, IEEE 754 single format
    const qint8 miRESERVED1       = 0x08;     // 8  : **Reserved**
    const qint8 miDOUBLE          = 0x09;     // 9  : 64-bit, IEEE 754 double format
    const qint8 miRESERVED2       = 0x0A;     // 10 : **Reserved**
    const qint8 miRESERVED3       = 0x0B;     // 11 : **Reserved**
    const qint8 miINT64           = 0x0C;     // 12 : 64-bit, signed
    const qint8 miUINT64          = 0x0D;     // 13 : 64-bit, unsigned
    const qint8 miMATRIX          = 0x0E;     // 14 : MATLAB array
    const qint8 miCOMPRESSED      = 0x0F;     // 15 : Compressed Data
    const qint8 miUTF8            = 0x010;    // 16 : Unicode UTF-8 Encoded Character Data
    const qint8 miUTF16           = 0x011;    // 17 : Unicode UTF-16 Encoded Character Data
    const qint8 miUTF32           = 0x012;    // 18 : Unicode UTF-32 Encoded Character Data
    QMap<qint8, qint8> m_mapSizeOfDataType;

    // Flags:   3rd byte in Tag of Array Flags SubElement (complex: 5th-bit, global: 6th-bit, logical: 7th-bit)
    // Classes: 4th byte in Tag of Array Flags SubElement
    const qint8 mxCELL_CLASS      = 0x01;     // 1  : Cell array
    const qint8 mxSTRUCT_CLASS    = 0x02;     // 2  : Structure
    const qint8 mxOBJECT_CLASS    = 0x03;     // 3  : Object
    const qint8 mxCHAR_CLASS      = 0x04;     // 4  : Character array
    const qint8 mxSPARSE_CLASS    = 0x05;     // 5  : Sparse array
    const qint8 mxDOUBLE_CLASS    = 0x06;     // 6  : Double precision array
    const qint8 mxSINGLE_CLASS    = 0x07;     // 7  : single precision array
    const qint8 mxINT8_CLASS      = 0x08;     // 8  : 8-bit, signed integer
    const qint8 mxUINT8_CLASS     = 0x09;     // 9  : 8-bit, unsigned integer
    const qint8 mxINT16_CLASS     = 0x0A;     // 10 : 16-bit, signed integer
    const qint8 mxUINT16_CLASS    = 0x0B;     // 11 : 16-bit, unsigned integer
    const qint8 mxINT32_CLASS     = 0x0C;     // 12 : 32-bit, signed integer
    const qint8 mxUINT32_CLASS    = 0x0D;     // 13 : 32-bit, unsigned integer
    const qint8 mxINT64_CLASS     = 0x0E;     // 14 : 64-bit, signed integer
    const qint8 mxUINT64_CLASS    = 0x0F;     // 15 : 64-bit, unsigned integer

    // Decoder functions: using cast to convert binary data to desired formats.
    static inline double miINT8Decoder(const QByteArray &byteArray) {
        return *(qint8*)(byteArray.constData()); // using cast to convert int8 to double.
    }
    static inline double miUINT8Decoder(const QByteArray &byteArray) {
        return *(quint8*)(byteArray.constData());
    }
    static inline double miINT16Decoder(const QByteArray &byteArray){
        return *(qint16*)(byteArray.constData());
    }
    static inline double miUINT16Decoder(const QByteArray &byteArray){
        return *(quint16*)(byteArray.constData());
    }
    static inline double miINT32Decoder(const QByteArray &byteArray){
        return *(qint32*)(byteArray.constData());
    }
    static inline double miUINT32Decoder(const QByteArray &byteArray){
        return *(quint32*)(byteArray.constData());
    }
    static inline double miDOUBLEDecoder(const QByteArray &byteArray){
        return *(double*)(byteArray.constData());
    }
    static inline double miSINGLEDecoder(const QByteArray &byteArray){
        return *(float*)(byteArray.constData());
    }
    static inline double miINT64Decoder(const QByteArray &byteArray){
        return *(qint64*)(byteArray.constData());
    }
    static inline double miUINT64Decoder(const QByteArray &byteArray){
        return *(quint64*)(byteArray.constData());
    }

signals:
    //=====================================================================================================================
    /**
     * @brief sig_updateVariableInfo
     * @param sVarInfo
     */
    void sig_updateVariableInfo(const QString &sVarInfo);
};
} // NAMESPACE

#endif // MATPARSER_H
