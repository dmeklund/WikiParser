/* Copyright (c) 2018 Peter Kondratyuk. All Rights Reserved.
*
* You may use, distribute and modify the code in this file under the terms of the MIT Open Source license, however
* if this file is included as part of a larger project, the project as a whole may be distributed under a different
* license.
*
* MIT license:
* Permission is hereby granted, free of charge, to any person obtaining a copy of this software and associated
* documentation files (the "Software"), to deal in the Software without restriction, including without limitation
* the rights to use, copy, modify, merge, publish, distribute, sublicense, and/or sell copies of the Software, and
* to permit persons to whom the Software is furnished to do so, subject to the following conditions:
*
* The above copyright notice and this permission notice shall be included in all copies or substantial portions
* of the Software.
*
* THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED
* TO THE WARRANTIES OF MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL
* THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION OF
* CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS
* IN THE SOFTWARE.
*/

#include <thread>
#include "wiki_qt_parser.h"
#include "CommonUtility.h"

Wiki_Qt_Parser::Wiki_Qt_Parser()
	: exeDir(boost::dll::program_location().parent_path().string() ),
	parser(exeDir + "pdata.cfg")
{
	//The file where the settings are saved from last time
	savableFile		= exeDir + "wps.cfg";

	//Set up file names used during parsing - except input file and directory, which are stored in "savable"
	xmlFile			= "articles_in_xml.xml";
	iiaFile			= "ADiia.ari64";
	pageIndexFile	= "pindex.cust";
	plainTextFile	= "articles_in_plain_text.txt";
	reportFile		= "parse_report.txt";
	redirectFile	= "redirects.txt";
	artTitleFile	= "article_titles.txt";

	//Pass file names to parser and set some parser options
	parser.SetXmlFileName(xmlFile);
	parser.SetIiaFileName(iiaFile);
	parser.SetPageIndexFileName(pageIndexFile);

	//Prepended to XML to make it legal XML
	//We'll need to append </pages> at the end after the parsing is done
	parser.SetPrependToXML("<?xml version=\"1.0\" encoding=\"UTF-8\" ?><pages>");

	//Number of articles for test runs and the number to show in the progress bar
	numArtsInTest = 100;
	numArtsInProgBar = 20000;

	//Number of cores to use
	numCores = std::thread::hardware_concurrency();
	if(numCores <= 0) numCores = 2;

	numCoresMinOne = numCores - 1;
	if(numCoresMinOne <= 0) numCoresMinOne = 1;

	numOtherCoresDefault = 2;
	if(numCores < 2) numOtherCoresDefault = 1;


	//Aux variable to show loading of the next data chunk into memory
	numDotsInProg = 1;

	//Parser options
	parser.SetShortReport(true);

	//Attempt to load all savable dialog data and show it in the dialog
//	Load();

	//State machine
	parserRunning = false;
	writerRunning = false;
	stopFlag = false;
}

Wiki_Qt_Parser::~Wiki_Qt_Parser()
{
	//Save the options on exit
//	Save();
}

//Load all savable data from the dialog
//void Wiki_Qt_Parser::Load()
//{
//	bool res = savable.Load(savableFile);
//	if(!res)		//Load unsuccessful - probably no such file, yet
//	{
//		//Load default options
//		savable.inputFile				= "";
//		savable.outputDir				= "";
//		savable.checkTest				= false;
//		savable.checkDiscardLists		= true;
//		savable.checkDiscardDisambigs	= true;
//		savable.checkDiscardCaptions	= false;
//		savable.checkMarkArticles		= true;
//		savable.checkMarkSections		= true;
//		savable.checkMarkCaptions		= true;
//
//		savable.radioCoresMinus1		= true;
//		savable.radioAllCores			= false;
//		savable.radioOtherCores			= false;
//		savable.numOtherCores			= 2;
//	}
//
//	//Show all loaded options in the dialog
//	ShowInputFile();
//	ShowOutputDir();
//}

////Save all savable data from the dialog
//void Wiki_Qt_Parser::Save()
//{
//	//No need to worry about input file and output directory, they are already in savable
//	savable.checkTest				= ui.checkTestRun->isChecked();
//	savable.checkDiscardLists		= ui.checkDiscardListPages->isChecked();
//	savable.checkDiscardDisambigs	= ui.checkDiscardDisambigs->isChecked();
//	savable.checkDiscardCaptions	= ui.checkSkipImageCaptions->isChecked();
//	savable.checkMarkArticles		= ui.checkMarkArticles->isChecked();
//	savable.checkMarkSections		= ui.checkMarkSections->isChecked();
//	savable.checkMarkCaptions		= ui.checkMarkCaptions->isChecked();
//
//	savable.radioCoresMinus1		= ui.radioAllCoresMinOne->isChecked();
//	savable.radioAllCores			= ui.radioAllCores->isChecked();
//	savable.radioOtherCores			= ui.radioOtherCores->isChecked();
//	savable.numOtherCores			= ui.editOtherCores->text().toInt();
//
//	savable.Save(savableFile);
//}


void Wiki_Qt_Parser::BnStartClicked()
{
	directory = savable.outputDir;

	//State machine
	parserRunning = true;
	writerRunning = false;
	stopFlag = false;


	//Figure out the number of threads
	int numThreads = numCoresMinOne;

	//How many pages to parse
	int pagesToParse = 1000000000;	//1 billion
	if(fTestRun) pagesToParse = numArtsInTest;

	//Open the streams from the input file
	instream.close();
	streambuf.reset();

	instream.open(savable.inputFile.c_str(),std::ios::binary | std::ios::in);

	if(!instream)
	{
		return;
	}
		
//    if(fileString.Right(8) == ".xml.bz2") streambuf.push(boost::iostreams::bzip2_decompressor());
    streambuf.push(instream);

	//Clear output stream
	parserReport.clear();
	parserReport.str("");

	//Set parser options
//	parser.SetDiscardLists(ui.checkDiscardListPages->isChecked());
//	parser.SetDiscardDisambigs(ui.checkDiscardDisambigs->isChecked());
	parser.SetWritePageIndex(false);

	//Tell the parser the input file name for reporting purposes
	parser.SetInputFileForReport(savable.inputFile);

//	//Set writer options
//	writer.SetSkipImCaptions(ui.checkSkipImageCaptions->isChecked());
//	writer.SetMarkArticles(ui.checkMarkArticles->isChecked());
//	writer.SetMarkSections(ui.checkMarkSections->isChecked());
//	writer.SetMarkCaptions(ui.checkMarkCaptions->isChecked());

	//Start parse asynchronously
	parser.Parse(	&streambuf,
					numThreads,
					directory,
					parserReport,
					pagesToParse,
					false);		//Asynchronous - will launch a wrapper thread and return immediately

	//Record start time
	stopwatch.SetTimerZero(0);
}


//The parser and writer have both finished
//We need to finalize the parse
void Wiki_Qt_Parser::OnTimerFinalize()
{
	//String with time taken
	int sec = (int)stopwatch.GetCurTime(0);
	int h, m, s;
	CommonUtility::SecondsToHMS(sec,h,m,s);
	BString timeString;
	timeString.Format("%0.2ih : %0.2im : %0.2is",h,m,s);

	//End parse message
//	QString message;
//	if(stopFlag) message = QString("Parse interruped at ") + timeString.c_str() + ".";
//	else message = QString("Parse completed in ") + timeString.c_str() + ".";
		
	//Get writer report
	writer.Report(parserReport);

	//Save report
	std::ofstream reportStream(directory + reportFile, std::ios::binary);
	BString repString = parserReport.str();
	reportStream.write(repString,repString.GetLength());

	//Finilize the XML file:
	//Append the </pages> tag at the end
	{
		std::ofstream  xmlStream(directory + xmlFile, std::ios::binary | std::ios::app);	//append
		xmlStream << "</pages>";
	}

	//State machine back to init state
	parserRunning = false;
	writerRunning = false;
	stopFlag = false;
}


//After parser is done, we can extract the redirects and article titles from the page index
void Wiki_Qt_Parser::ProcessPageIndex(PageIndex& index)
{
	index.artDisambigUrls.WriteStrings(directory + artTitleFile);

	index.redirectFrom += "\t\t-->\t\t";
	index.redirectFrom += index.redirectTo;

	index.redirectFrom.WriteStrings(directory + redirectFile);
}



