#include <iostream>
#include <string.h>
#include <fstream>
#include <iostream>
#include <assert.h>

using namespace std;

//Constants
#define METHOD_NUM 9
#define SCHEME "http://"
#define ARGUMENTS_NUM 2

//Error codes
#define NO_SUCH_HEADER "Paris"
#define FAILED_TO_EXTRACT_URL "Rome"

//Functions
void summarizeMessage(const string&);
int headersNum(ifstream*);
bool isMethod(const string&);
int NthApp(const string&, char, int=1);
string NthWord(const string&, int=1);
string extractURL(ifstream*);
string getHeaderLine(ifstream*, const string&);



/**
 * This program gets a path to a file that contains a legal HTTP message and summarize it
 * @param argc: The programs takes one argument
 * @param argv: The one argument is the path to the file that contains the HTTP message
 */
int main(int argc, char *argv[]) {

    //We assume that the user will enter an adequate number of arguments
    assert(argc==ARGUMENTS_NUM);
    assert(argv[1] !=NULL);

    //the path is in the first argument
    const string& path(argv[1]);
    summarizeMessage(path);
}



/**
 * This function summarize the message stored in a given file
 * @param filepath: The path to the given file
 */
void summarizeMessage(const string& filepath){
    //Opining the file
    ifstream* msg = new ifstream;
    msg->open(filepath);
    if(!msg->is_open()){
        cout<<"Some error occurred "<<endl;
        return;
    }
    //Computing the values that we will need regardless of the message type (request/respond):

    //Computing number of headers in the given file:
    int headers= headersNum(msg);
    if(headers == -1){
        cout<<"You sent an empty message"<<endl;
        return;
    }

    //Computing the number of bytes in the message body:
    //After calling headersNum(), the file pointer stands at the start of the message body.
    //So the body size is the distance between the current file pointer "location" and the end of the file
    auto bodyBase = msg->tellg();
    msg->seekg(0,ios_base::end);
    auto fileEnd = msg->tellg();
    auto bodySize = fileEnd - bodyBase;


    //Now we need to check the type of the message
    //We do that by checking if the message starts with a name of a method
    msg->seekg(0,ios_base::beg);
    string firstLine;
    getline(*msg,firstLine);
    string firstWord = NthWord(firstLine);
    if(!isMethod(firstWord)){
        //If we get here then the message is a respond message.
        //The status code and phrase are what comes after the first word in the first line
        string status = firstLine.substr((unsigned int)(NthApp(firstLine,' ')+1));
        cout<<status<<' ';
    }else{
        //If we get here then the message is a request message.
        //URL is the only needed value that is yet to be computed.
        string url = extractURL(msg);
        if(url == FAILED_TO_EXTRACT_URL){
            cout<<"The message you sent is not legal"<<endl;
        }
        cout<<firstWord<<' '<<url<<' ';
    }

    //Adding the last piece of information to our humble summary.
    cout<<'('<<headers<<" headers, "<<bodySize<<" bytes)"<<endl;

    msg->close();
    delete msg;
}

/**
 * This function finds the line correspanding to a given header specifier
 * @param msg: The file that contains the message
 * @param headerName: The header specifier name.
 * @return The line that contains the header specifier we got.
 * If such a line dont exist, we return the error code NO_SUCH_HEADER.
 */
string getHeaderLine(ifstream* msg, const string& headerName){
    msg->seekg(0,ios_base::beg);
    string curLine;
    getline(*msg,curLine); //skipping the first line
    while(getline(*msg,curLine)){
        string curHeaderName = NthWord(curLine);
        curHeaderName = curHeaderName.substr(0,curHeaderName.length()-1); //omit the ':' character
        if(curHeaderName == headerName){
            return curLine;
        }
    }
    return NO_SUCH_HEADER;
}


/**
 *This function mission is to extract the message URL
 * @param msg: The file that contains the message
 * @return The URL of the message
 */
string extractURL(ifstream* msg){
    msg->seekg(0,ios_base::beg);
    string firstLine;
    getline(*msg,firstLine);
    string URI = NthWord(firstLine,2);
    string URL;
    if(URI == "*"){
        //if we get here then the message does not apply to a particular resource
        URL = "*";
    }else if(URI.at(0)=='/'){
        //if we get here then the URI of the message is given in "abs_path" form
        string hostHeader = getHeaderLine(msg,"Host");
        if(hostHeader == NO_SUCH_HEADER){
            return FAILED_TO_EXTRACT_URL;
        }
        string hostValue = NthWord(hostHeader,2);
        URL= SCHEME + hostValue + URI;
    }else{
        //if we get here then the URI of the message is given in "absoluteURI" or in "authority" form.
        URL = URI;
    }
    return URL;

}

/**
 * Finds the Nth appearance of a character in a string
 * @param str: The string
 * @param c: The character
 * @param N: by default we will search for the first appearance
 * @return The index of the Nth appearance of "c" in "str"
 */
int NthApp(const string& str, char c, int N){
    for(unsigned int i=0; i<str.length(); ++i){
        if(str.at(i) == c)
            N--;
        if(N==0)
            return i;
    }
    return -1;
}
/**
 * Checks if a given string describes an HTTP request method
 * @param word: the string
 * @return true or false in accordance to the word
 */
bool isMethod(const string& word){
    string methodes[METHOD_NUM]={"GET","HEAD","POST","PUT","DELETE","CONNECT","OPTIONS","TRACE","PATCH"};
    for (int i=0; i<METHOD_NUM; ++i){
        if(methodes[i] == word) return true;
    }
    return false;
}

/**
 * Counts the number of headers in a given HTTP message
 * @param File : The file that contains the HTTP message
 * @return The number of headers in the HTTP message stored in "file"
 */
int headersNum(ifstream* file){
    int counter=-1;
    string line;
    while(getline(*file,line)){
        if(line == ""){
            break;
        }
        counter++;
    }
    file->clear();
    return counter;
}


/**
 * Finds the Nth word in a string
 * @param str: The string
 * @param n: by default we will search for the first word
 * @return The "N"th word
 */
string NthWord(const string& str, int N){
    if(N==1){
        auto endOfFirstWord = str.find(' ');
        string firstWord = str.substr(0,endOfFirstWord);
        return firstWord;
    }else{
        int start = NthApp(str,' ',N-1);
        int end = NthApp(str,' ',N);
        if(end != -1){
            string word = str.substr((unsigned int)(start+1), (unsigned int)(end-start-1));
            return word;
        } else{
            string word = str.substr((unsigned int)(start+1));
            return word;
        }
    }
}



