//
// printer.cc
//

// This file contains the interface for the Printer class and some
// functions that will be used by both the SM and QL components.

#include <cstdio>
#include <cstring>
#include <cstdlib>
#include <iostream>
#include <fstream>
#include <exception>
#include <string>
#include <boost/asio.hpp>
#include "printer.h"
#include "parser.h"

void writeToFile(const char* data);
// string globalAscii="";
using namespace std;

//
// void Spaces(int maxLength, int printedSoFar)
//
// This method will output some spaces so that print entry will align everythin
// nice and neat.
//
void Spaces(int maxLength, int printedSoFar)
{
    for (int i = printedSoFar; i < maxLength; i++)
        cout << " ";
}

//
// ------------------------------------------------------------------------------
//
Printer::Printer(const Tuple& t)
{
  Init(t.GetAttributes(), t.GetAttrCount());
}

//
// Printer
//
// This class handles the printing of tuples.
//
//  DataAttrInfo - describes all of the attributes. Defined
//      within sm.h
//  attrCount - the number of attributes
//
Printer::Printer(const DataAttrInfo *attributes_, const int attrCount_)
{
  Init(attributes_, attrCount_);
}

boost::asio::ip::tcp::socket* Printer::enableConnection(char *host, char *port)
{
  std::cout << "Conn started" << std::endl;
  boost::asio::io_service io_service;

  boost::asio::ip::tcp::socket *s = new boost::asio::ip::tcp::socket(io_service);
  boost::asio::ip::tcp::resolver resolver(io_service);
  try {
    boost::asio::connect(*s, resolver.resolve({host, port}));
  }
  catch (boost::system::system_error const& e)
  {
    std::cout << "Warning: could not connect : " << e.what() << std::endl;
  }
  std::cout << "Conn established" << std::endl;
  return s;
}

size_t Printer::writeRead(boost::asio::ip::tcp::socket *s, char request[], char reply[])
{
  std::cout << "Write read trying to write.." << std::endl;
  boost::system::error_code error;
  size_t request_length = std::strlen(request);
    boost::asio::write(*s, boost::asio::buffer(request, request_length));
  std::cout << "Success written" << std::endl;
  std::cout << "Waiting to read.." << std::endl;
  // size_t reply_length = boost::asio::read(*s,
  //       boost::asio::buffer(reply, request_length*4));
  // size_t len;
  size_t reply_length = s->read_some(boost::asio::buffer(reply, request_length), error);
  reply[reply_length] = '\0';
  std::cout << "Read " << reply << std::endl;
  return reply_length;
}

size_t Printer::sendRecvFrom(char *host, char *port, 
                           char request[], char reply[])
{
  std::cout << "sendrecv starting" << std::endl;
  boost::asio::ip::tcp::socket *s = enableConnection(host, port);
  std::cout << "sendrecv finish; trying writeRead" << std::endl;
  size_t rn = writeRead(s, request, reply);
  std::cout << "writeread fin, sendrecv fin" << std::endl;
  return rn;
}



// void Printer::session(boost::asio::ip::tcp::socket sock)
// {
//   try
//   {
//     for (;;)
//     {
//       char data[max_length];
//       char reply_msg[max_length];

//       boost::system::error_code error;
//       size_t length = sock.read_some(boost::asio::buffer(data), error);
//       if (error == boost::asio::error::eof)
//         break; // Connection closed cleanly by peer.
//       else if (error)
//         throw boost::system::system_error(error); // Some other error.
      
      
//       boost::asio::write(sock, boost::asio::buffer(data, length));
//     }
//   }
//   catch (std::exception& e)
//   {
//     std::cerr << "Exception in thread: " << e.what() << "\n";
//   }
// }

// void Printer::server(boost::asio::io_service& io_service, unsigned short port)
// {
//   boost::asio::ip::tcp::acceptor a(io_service, 
//     boost::asio::ip::tcp::endpoint(boost::asio::ip::tcp::v4(), port));
//   for (;;)
//   {
//     boost::asio::ip::tcp::socket sock(io_service);
//     a.accept(sock);
//     std::thread(session, std::move(sock)).detach();
//   }
// }

// void Printer::spawnServer(unsigned short port)
// {
//   boost::asio::io_service io_service;
//   server(io_service, port);
// }

void Printer::Init(const DataAttrInfo *attributes_, const int attrCount_)
{
    attrCount = attrCount_;
    attributes = new DataAttrInfo[attrCount];

    for (int i=0; i < attrCount; i++) {
        attributes[i] = attributes_[i];
        // cout << "Printer::init i, offset " << attributes[i].offset << endl;
    }
    // Number of tuples printed
    iCount = 0;

    // Figure out what the header information will look like.  Normally,
    // we can just use the attribute name, but if that appears more than
    // once, then we should use "relation.attribute".

    // this line broke when using CC
    // changing to use malloc and free instead of new and delete
    // psHeader = (char **) new (char *)[attrCount];
    psHeader = (char**)malloc(attrCount * sizeof(char*));

    // Also figure out the number of spaces between each attribute
    spaces = new int[attrCount];

    for (int i=0; i < attrCount; i++ ) {
        // Try to find the attribute in another column
        int bFound = 0;
        psHeader[i] = new char[MAXPRINTSTRING];
        memset(psHeader[i],0,MAXPRINTSTRING);

        for (int j=0; j < attrCount; j++)
            if (j != i &&
                strcmp(attributes[i].attrName,
                       attributes[j].attrName) == 0) {
                bFound = 1;
                break;
            }

        if (bFound)
            sprintf(psHeader[i], "%s.%s",
                    attributes[i].relName, attributes[i].attrName);
        else
            strcpy(psHeader[i], attributes[i].attrName);

        if(attributes[i].func != NO_F) {
          char * foo = "NO_F";
          if(attributes[i].func == MAX_F)
            foo = "MAX";
          else if(attributes[i].func == MIN_F)
            foo = "MIN";
          else if(attributes[i].func == COUNT_F)
            foo = "COUNT";
          else if(attributes[i].func == AVG_F)
            foo = "AVG";
          else if(attributes[i].func == SUM_F)
            foo = "SUM";
          char * copy = strdup(psHeader[i]);
          sprintf(psHeader[i], "%s(%s)",
                  foo, copy);
          free(copy);
        }

        if (attributes[i].attrType==STRING)
            spaces[i] = mmin(attributes[i].attrLength, MAXPRINTSTRING);
        else
            spaces[i] = mmax(12, strlen(psHeader[i]));

        // We must subtract out those characters that will be for the
        // header.
        spaces[i] -= strlen(psHeader[i]);

        // If there are negative (or zero) spaces, then insert a single
        // space.
        if (spaces[i] < 1) {
            // The psHeader will give us the space we need
            spaces[i] = 0;
            strcat(psHeader[i]," ");
        }
    }
}


//
// Destructor
//
Printer::~Printer()
{
    for (int i = 0; i < attrCount; i++)
        delete [] psHeader[i];

    delete [] spaces;
    //delete [] psHeader;
    free (psHeader);
    delete [] attributes;
}

//
// PrintHeader
//
void Printer::PrintHeader( ostream &c ) const
{
    int dashes = 0;
    int iLen;
    int i,j;

    for (i = 0; i < attrCount; i++) {
        // Print out the header information name
        c << psHeader[i];
        iLen = strlen(psHeader[i]);
        dashes += iLen;

        for (j = 0; j < spaces[i]; j++)
            c << " ";

        dashes += spaces[i];
    }

    c << "\n";
    for (i = 0; i < dashes; i++) c << "-";
    c << "\n";
}

//
// PrintFooter
//
void Printer::PrintFooter(ostream &c) const
{
    c << "\n";
    c << iCount << " tuple(s).\n";
}

//
// Print
//
//  data - this is an array of void *.  This print routine is used by
//  the QL Layer.
//
//  Unfortunately, this is essentially the same as the other Print
//  routine.
//
void Printer::Print(ostream &c, const void * const data[])
{
    char str[MAXPRINTSTRING], strSpace[50];
    int i, a;
    float b;

    // Increment the number of tuples printed
    iCount++;

    for (i = 0; i<attrCount; i++) {
        if (attributes[i].attrType == STRING) {
            // We will only print out the first MAXNAME+10 characters of
            // the string value.
            memset(str,0,MAXPRINTSTRING);

            if (attributes[i].attrLength>MAXPRINTSTRING) {
                strncpy(str, (char *)data[i], MAXPRINTSTRING-1);
                str[MAXPRINTSTRING-3] ='.';
                str[MAXPRINTSTRING-2] ='.';

                // writeToFile(str);

                c << str;
                Spaces(MAXPRINTSTRING, strlen(str));
            } else {
                strncpy(str, (char *)data[i], attributes[i].attrLength);

                // writeToFile(str);

                c << str;
                if (attributes[i].attrLength < (int) strlen(psHeader[i]))
                    Spaces(strlen(psHeader[i]), strlen(str));
                else
                    Spaces(attributes[i].attrLength, strlen(str));
            }
        }
        if (attributes[i].attrType == INT) {
            memcpy (&a, data[i], sizeof(int));
            sprintf(strSpace, "%d",a);

            // writeToFile(str);

            c << a;
            if (strlen(psHeader[i]) < 12)
                Spaces(12, strlen(strSpace));
            else
                Spaces(strlen(psHeader[i]), strlen(strSpace));
        }
        if (attributes[i].attrType == FLOAT) {
            memcpy (&b, data[i], sizeof(float));
            sprintf(strSpace, "%f",b);
            c << strSpace;
            if (strlen(psHeader[i]) < 12)
                Spaces(12, strlen(strSpace));
            else
                Spaces(strlen(psHeader[i]), strlen(strSpace));
        }
    }
    c << "\n";
}

void Printer::Print(std::ostream &c, const Tuple& t)
{
  const char * data;
  t.GetData(data);

  cout << "PRINT TUPLE : " << t << endl;
  //get the tuples in ascii form
  string ascii;
  t.GetDataInAscii(ascii);
  cout<<"ASCII first= " << ascii << endl;
  char sndm[10000], repl[10000];
  strcpy(sndm, ascii.c_str());
  char *ipa = "10.50.42.99";
  char *por = "8888";
  // char *ipas = ipa.c_str();
  // char *pors = por.c_str();
  size_t s = sendRecvFrom(ipa, por, sndm, repl);
  repl[s] = '\0';
  // globalAscii += "(";
  // globalAscii += ascii + "\n";
  // globalAscii += ")|";
  cout<<"ASCII : " << ascii << endl;
  cout<<"Rec from python" << string(repl) << endl;
  // cout<<"GLOBAL ASCII : "<< globalAscii << endl;

  /*
    method to send each tuple over the socket
  */
  // sendResultInAscii(ascii);

  // print the tuples on console
  Print(c, data);
}

//
// Print
//
//  data - the actual data for the tuple to be printed
//
//  The routine tries to make things line up nice, however no
//  attempt is made to keep the tuple constrained to some number of
//  characters.
//

void Printer::Print(ostream &c, const char * const data)
{
    char str[MAXPRINTSTRING], strSpace[50];
    int i, a;
    float b;

    if (data == NULL)
        return;

    // Increment the number of tuples printed
    iCount++;

    for (i = 0; i<attrCount; i++) {
      // cout << "[Printer::Print i, offset " << attributes[i].offset << "]";

        if (attributes[i].attrType == STRING) {
            // We will only print out the first MAXNAME+10 characters of
            // the string value.
            memset(str,0,MAXPRINTSTRING);

            if (attributes[i].attrLength>MAXPRINTSTRING) {
                strncpy(str, data+attributes[i].offset, MAXPRINTSTRING-1);
                str[MAXPRINTSTRING-3] ='.';
                str[MAXPRINTSTRING-2] ='.';

                // writeToFile(str);

                c << str;
                Spaces(MAXPRINTSTRING, strlen(str));
            } else {
                strncpy(str, data+attributes[i].offset, attributes[i].attrLength);

                // writeToFile(str);

                c << str;
                if (attributes[i].attrLength < (int) strlen(psHeader[i]))
                    Spaces(strlen(psHeader[i]), strlen(str));
                else
                    Spaces(attributes[i].attrLength, strlen(str));
            }
        }
        if (attributes[i].attrType == INT) {
            memcpy (&a, (data+attributes[i].offset), sizeof(int));
            sprintf(strSpace, "%d",a);

            // writeToFile(str);

            c << a;
            if (strlen(psHeader[i]) < 12)
                Spaces(12, strlen(strSpace));
            else
                Spaces(strlen(psHeader[i]), strlen(strSpace));
        }
        if (attributes[i].attrType == FLOAT) {
            memcpy (&b, (data+attributes[i].offset), sizeof(float));
            sprintf(strSpace, "%f",b);

            // writeToFile(str);

            c << strSpace;
            if (strlen(psHeader[i]) < 12)
                Spaces(12, strlen(strSpace));
            else
                Spaces(strlen(psHeader[i]), strlen(strSpace));
        }
    }
    c << "\n";
}

void writeToFile(const char * data)
{
    cout<<"writing file\n";
    try{
        // fstream outputfile;
        // outputfile.open("TESTYTESY.txt", fstream::out);
        // outputfile << data <<endl;

        FILE* fptr=fopen("test.txt","w");
        if(fptr!=NULL)
        {
            char* s="vinaychetanvinay";
            cout<<"file not null\n";
            fputs(s, fptr);
            fclose(fptr);
        }
    }
    catch(exception& e)
    {
        cout << e.what() <<endl;
    }

}
