//=============================================================================================================
/**
* @file     main.cpp
* @author   Christoph Dinh <chdinh@nmr.mgh.harvard.edu>;
*           Lorenz Esch <lorenz.esch@tu-ilmenau.de>;
*           Matti Hamalainen <msh@nmr.mgh.harvard.edu>
* @version  1.0
* @date     July, 2012
*
* @section  LICENSE
*
* Copyright (C) 2012, Christoph Dinh and Matti Hamalainen. All rights reserved.
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
* @brief    Example of a clusteredInverse application
*
*/


//*************************************************************************************************************
//=============================================================================================================
// INCLUDES
//=============================================================================================================

#include <fs/label.h>
#include <fs/surface.h>
#include <fs/surfaceset.h>
#include <fs/annotationset.h>

#include <fiff/fiff_evoked.h>
#include <mne/mne_sourceestimate.h>
#include <inverse/minimumNorm/minimumnorm.h>

#include <utils/mnemath.h>

#include <iostream>


//*************************************************************************************************************
//=============================================================================================================
// QT INCLUDES
//=============================================================================================================

#include <QApplication>
#include <QCommandLineParser>
#include <QSet>


//*************************************************************************************************************
//=============================================================================================================
// USED NAMESPACES
//=============================================================================================================

using namespace MNELIB;
using namespace FSLIB;
using namespace FIFFLIB;
using namespace INVERSELIB;
using namespace UTILSLIB;


//*************************************************************************************************************
//=============================================================================================================
// MAIN
//=============================================================================================================

//=============================================================================================================
/**
* The function main marks the entry point of the program.
* By default, main has the storage class extern.
*
* @param [in] argc (argument count) is an integer that indicates how many arguments were entered on the command line when the program was started.
* @param [in] argv (argument vector) is an array of pointers to arrays of character objects. The array objects are null-terminated strings, representing the arguments that were entered on the command line when the program was started.
* @return the value that was set to exit() (which is 0 if exit() is called via quit()).
*/
int main(int argc, char *argv[])
{
    QApplication app(argc, argv);

    // Command Line Parser
    QCommandLineParser parser;
    parser.setApplicationDescription("Start clusteredInverse example");
    parser.addHelpOption();
    QCommandLineOption fwdFileOption("fwd", "Path to the forward solution <file>.", "file", "./MNE-sample-data/MEG/sample/sample_audvis-meg-eeg-oct-6-fwd.fif");
    QCommandLineOption evokedFileOption("ave", "Path to the evoked/average <file>.", "file", "./MNE-sample-data/MEG/sample/sample_audvis-ave.fif");
    QCommandLineOption snrOption("snr", "The <snr> value used for computation.", "snr", "1.0");//3.0f;//0.1f;//3.0f;
    QCommandLineOption annotOption("annotType", "Annotation type <type>.", "type", "aparc.a2009s");
    QCommandLineOption subjectOption("subject", "Selected subject <subject>.", "subject", "sample");
    QCommandLineOption subjectPathOption("subjectPath", "Selected subject path <subjectPath>.", "subjectPath", "./MNE-sample-data/subjects");
    QCommandLineOption hemiOption("hemi", "Selected hemisphere <hemi>.", "hemi", "2");
    QCommandLineOption evokedSetNoOption("setNo", "The average <setNo> to pick (must match ID or comment).", "setNo", "1");

    parser.addOption(fwdFileOption);
    parser.addOption(evokedFileOption);
    parser.addOption(snrOption);
    parser.addOption(annotOption);
    parser.addOption(subjectOption);
    parser.addOption(subjectPathOption);
    parser.addOption(evokedSetNoOption);

    parser.process(app);

    //Parse inputs
    QFile t_fileFwd(parser.value(fwdFileOption));
    QFile t_fileEvoked(parser.value(evokedFileOption));

    // Load data
    QPair<QVariant, QVariant> baseline(QVariant(), 0);
    FiffEvoked evoked(t_fileEvoked, parser.value(evokedSetNoOption).toInt(), baseline);
    if(evoked.isEmpty())
        return 1;

    std::cout << "evoked first " << evoked.first << "; last " << evoked.last << std::endl;

    MNEForwardSolution t_Fwd(t_fileFwd);
    if(t_Fwd.isEmpty())
        return 1;

    AnnotationSet t_annotationSet (parser.value(subjectOption), parser.value(hemiOption).toInt(), parser.value(annotOption), parser.value(subjectPathOption));

    //
    // Cluster forward solution;
    //
    MNEForwardSolution t_clusteredFwd = t_Fwd.cluster_forward_solution(t_annotationSet, 20);//40);

    return app.exec();//1;
}
