//=============================================================================================================
/**
* @file     correlation.cpp
* @author   Lorenz Esch <Lorenz.Esch@tu-ilmenau.de>;
*           Matti Hamalainen <msh@nmr.mgh.harvard.edu>
* @version  1.0
* @date     January, 2018
*
* @section  LICENSE
*
* Copyright (C) 2018, Lorenz Esch and Matti Hamalainen. All rights reserved.
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
* @brief    Correlation class definition.
*
*/


//*************************************************************************************************************
//=============================================================================================================
// INCLUDES
//=============================================================================================================

#include "correlation.h"
#include "network/networknode.h"
#include "network/networkedge.h"
#include "network/network.h"


//*************************************************************************************************************
//=============================================================================================================
// QT INCLUDES
//=============================================================================================================

#include <QDebug>
#include <QElapsedTimer>
#include <QtConcurrent>


//*************************************************************************************************************
//=============================================================================================================
// Eigen INCLUDES
//=============================================================================================================

#include <unsupported/Eigen/FFT>


//*************************************************************************************************************
//=============================================================================================================
// USED NAMESPACES
//=============================================================================================================

using namespace CONNECTIVITYLIB;
using namespace Eigen;


//*************************************************************************************************************
//=============================================================================================================
// DEFINE GLOBAL METHODS
//=============================================================================================================


//*************************************************************************************************************
//=============================================================================================================
// DEFINE MEMBER METHODS
//=============================================================================================================

Correlation::Correlation()
{
}


//*************************************************************************************************************

Network Correlation::correlationCoeff(const QList<MatrixXd> &matDataList,
                                      const MatrixX3f& matVert)
{
    Network finalNetwork("Correlation");

    if(matDataList.empty()) {
        qDebug() << "Correlation::correlationCoeff - Input data is empty";
        return finalNetwork;
    }   

    //Create nodes
    int rows = matDataList.first().rows();
    RowVectorXf rowVert = RowVectorXf::Zero(3);

    for(int i = 0; i < rows; ++i) {
        if(matVert.rows() != 0 && i < matVert.rows()) {
            rowVert(0) = matVert.row(i)(0);
            rowVert(1) = matVert.row(i)(1);
            rowVert(2) = matVert.row(i)(2);
        }

        finalNetwork.append(NetworkNode::SPtr(new NetworkNode(i, rowVert)));
    }

    // Calculate connectivity matrix over epochs and average afterwards
    //double dScalingStep = 1.0/matDataList.size();
    //dataTemp.matInputData = dScalingStep * (i+1) * matDataList.at(i);

    QFuture<MatrixXd> resultMat = QtConcurrent::mappedReduced(matDataList,
                                                              compute,
                                                              reduce);
    resultMat.waitForFinished();

    MatrixXd matDist = resultMat.result();
    matDist /= matDataList.size();
    MatrixXd matWeight(1,1);

    //Add edges to network
    for(int i = 0; i < matDist.rows(); ++i) {
        for(int j = i; j < matDist.cols(); ++j) {
            matWeight << matDist(i,j);

            QSharedPointer<NetworkEdge> pEdge = QSharedPointer<NetworkEdge>(new NetworkEdge(i, j, matWeight));

            finalNetwork.getNodeAt(i)->append(pEdge);
            finalNetwork.getNodeAt(j)->append(pEdge);
            finalNetwork.append(pEdge);
        }
    }

    return finalNetwork;
}



//*************************************************************************************************************

MatrixXd Correlation::compute(const MatrixXd& inputData)
{
    MatrixXd matDist = MatrixXd::Zero(inputData.rows(), inputData.rows());
    RowVectorXd vecRow;
    int j;

    for(int i = 0; i < inputData.rows(); ++i) {
        vecRow = inputData.row(i);

        for(j = i; j < inputData.rows(); ++j) {
            matDist(i,j) += vecRow.dot(inputData.row(j))/vecRow.cols();
        }
    }

    return matDist;
}


//*************************************************************************************************************

void Correlation::reduce(MatrixXd &resultData,
                      const MatrixXd &data)
{
    if(resultData.rows() != data.rows() || resultData.cols() != data.cols()) {
        resultData.resize(data.rows(), data.cols());
        resultData.setZero();
    }

    resultData += data;
}
