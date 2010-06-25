// A sample application to extract all image attachments, i.e. attachments
// with any of the extensions gif, jpg or png.
// All image attachments are saved in the current working directory, with care
// taken not to overwrite any existing files.

#include <pstsdk/pst.h>
#include <fstream>
#include <iostream>
#include <sstream>
#include <string>

const std::wstring gifExtn = L"gif";
const std::wstring jpgExtn = L"jpg";
const std::wstring pngExtn = L"png";

void saveAttachment(const pstsdk::attachment& attch)
{
    std::wstring fname(attch.get_filename());
    // Not sure if this is safe or not
    std::string filenameFull(fname.begin(), fname.end());
    // OK to directly use rfind without checking the result as we have already
    // ensured that a '.' is present in the filename in processAttachment.
    size_t pos = filenameFull.rfind(".");
    std::string filenameBase(filenameFull.substr(0, pos));
    std::string filenameExtn(filenameFull.substr(pos));

    bool done = false;
    unsigned int i = 1;
    std::stringstream ss;
    ss << filenameFull;
    do
    {
        // Check if the file already exists
        std::ifstream testImgFile(ss.str().c_str(), std::ios::in | std::ios::binary);
        if(testImgFile.is_open())
        {
            testImgFile.close();
            // File with the name of current attachment already exits.
            // Do not overwrite the existing file, instead use a different name
            // for this file.
            ss.str(std::string());
            ss.clear();
            ss << filenameBase;
            ss << "(" << i++ << ")";
            ss << filenameExtn;
        }
        else
        {
            done = true;
        }
    } while(!done);

    std::cout << "Saving image attachment to '" << ss.str() << "'\n";
    std::ofstream imgFile(ss.str().c_str(), std::ios::out | std::ios::binary);
    imgFile << attch;
    imgFile.close();
}

void processAttachment(const pstsdk::attachment& attch)
{
    // Parse out the extension from the file name
    std::wstring filename = attch.get_filename();
    size_t dotPos = filename.find_last_of('.');
    // Only consider files with an extension
    if(dotPos != filename.npos)
    {
        // Extract the file extension
        std::wstring extn(filename, dotPos + 1, filename.length());

        // Shortcut:
        // Since we know that we are looking for 3 character extensions, reject
        // on basis of extension length before doing string comparision.
        // Might not hold true in the future and may need to be removed.
        if(extn.length() != 3)
        {
            return;
        }

        std::wstring lowerExtn(extn);
        // Convert to lower case for comparision purposes
        transform(extn.begin(), extn.end(), lowerExtn.begin(), tolower);
        // Only process certain, recognised image extensions
        if((lowerExtn == gifExtn) || (lowerExtn == jpgExtn) || (lowerExtn ==pngExtn))
        {
            saveAttachment(attch);
        }
    }
}

void processMessage(const pstsdk::message& msg)
{
    // Ensure that there is atleast one attachment to the current message
    if(msg.get_attachment_count() > 0)
    {
        for(pstsdk::message::attachment_iterator iter = msg.attachment_begin(); iter != msg.attachment_end(); ++iter)
        {
            processAttachment(*iter);
        }
    }
}

void traverseAllMessages(const pstsdk::pst& pstFile)
{
    // Iterate through all the folders in the given PST object.
    for(pstsdk::pst::folder_iterator i = pstFile.folder_begin(); i != pstFile.folder_end(); ++i)
    {
        pstsdk::folder currFolder = *i;

        // Process all messages in the current folder.
        if(currFolder.get_message_count() > 0)
        {
            for(pstsdk::folder::message_iterator j = currFolder.message_begin(); j != currFolder.message_end(); ++j)
            {
                processMessage(*j);
            }
        }
    }
}

void printUsage(const char* program)
{
    std::cout << "Usage\n";
    std::cout << "\t" << program << " [PST file]\n";
}

int main(int argc, char* argv[])
{
    if(argc != 2)
    {
        printUsage(argv[0]);
        exit(-1);
    }

    std::string pstF(argv[1]);
    // Check if the file exists
    std::ifstream pfile(argv[1]);
    if(pfile.is_open())
    {
        pfile.close();
    }
    else
    {
        std::cout << "The specified PST file argument is incorrect. "
                  << "Either it is not a file or the file does not exist\n";
        exit(-1);
    }
    std::wstring pstFile(pstF.begin(), pstF.end());
    pstsdk::pst myFile(pstFile);

    traverseAllMessages(myFile);
}
