//
// redbase.cc
//
// Author: Jason McHugh (mchughj@cs.stanford.edu)
//
// This shell is provided for the student.

#include <iostream>
#include <cstdio>
#include <cstring>
#include <unistd.h>
#include <fstream>
#include <iostream>
#include "redbase.h"
#include "rm.h"
#include "sm.h"
#include "ql.h"
#include "parser.h"
#include <cstdlib>
#include <iostream>
#include <thread>
#include <utility>
#include <boost/asio.hpp>

using namespace std;

static void PrintErrorExit(RC rc) {
  PrintErrorAll(rc);
  exit(rc);
}

using boost::asio::ip::tcp;
const int max_length = 1024;

FILE * inputFilePtr;

void triggerGen(char *dbname)
{
  RC rc;
  PF_Manager pfm;
  RM_Manager rmm(pfm);
  IX_Manager ixm(pfm);
  SM_Manager smm(ixm, rmm);
  QL_Manager qlm(smm, ixm, rmm);
  // open the database

  inputFilePtr = fopen("queryfile", "r");
  if(!inputFilePtr) {
    fprintf(stderr, "unable to open input file\n");
    exit(1); 
  }

  if ((rc = smm.OpenDb(dbname)))
    PrintErrorExit(rc);
  
  // freopen("queryfile", "r", stdin);
  // call the parser
  ofstream ofs;
  ofs.open("outputquery", std::ofstream::out | std::ofstream::trunc);
  ofs.close();
  RC parseRC = RBparse(pfm, smm, qlm);
  fclose(inputFilePtr);
  // close the database
  if ((rc = smm.CloseDb()))
    PrintErrorExit(rc);

  if(parseRC != 0)
    PrintErrorExit(parseRC);

  cout << "Bye.\n";
}

void write_to_file(char *fileName, char data[], size_t length)
{
  ofstream myfile;
  myfile.open (fileName);
  myfile << data << std::endl << "exit;" << std::endl;
  myfile.close();
}

void load_from_file(char *filename, char data[], int &length)
{
  ifstream file(filename, std::ios::binary | std::ios::ate);
  streamsize size = file.tellg();
  file.seekg(0, std::ios::beg);

  if (file.read(data, size))
  {
      /* worked! */
  }
  length = (int) size;
  file.close();

  // std::ifstream t(filename);
  // std::stringstream buffer;
  // buffer << t.rdbuf();
}

void session(tcp::socket sock)
{
  try
  {
    for (;;)
    {
      char data[max_length];
      char repldata[max_length*10];
      int repl_length;

      boost::system::error_code error;
      size_t length = sock.read_some(boost::asio::buffer(data), error);
      if (error == boost::asio::error::eof)
        break; // Connection closed cleanly by peer.
      else if (error)
        throw boost::system::system_error(error); // Some other error.
      
      char *filename = "queryfile";
      char *dbn = "test";
      char *outfile = "outputquery";
      write_to_file(filename, data, length);
      triggerGen(dbn);
      load_from_file("test/outputquery", repldata, repl_length);
      boost::asio::write(sock, boost::asio::buffer(repldata, repl_length));
    }
  }
  catch (std::exception& e)
  {
    std::cerr << "Exception in thread: " << e.what() << "\n";
  }
}

void server(boost::asio::io_service& io_service, unsigned short port)
{
  tcp::acceptor a(io_service, tcp::endpoint(tcp::v4(), port));
  for (;;)
  {
    tcp::socket sock(io_service);
    a.accept(sock);
    std::thread(session, std::move(sock)).detach();
  }
}
//
// main
//
int main(int argc, char *argv[])
{
  RC rc;

  // Look for 2 arguments.  The first is always the name of the program
  // that was executed, and the second should be the name of the
  // database.
  if (argc != 2) {
    cerr << "Usage: " << argv[0] << " port\n";
    exit(1);
  }

  try
  {
    boost::asio::io_service io_service;

    server(io_service, 8888);
  }
  catch (std::exception& e)
  {
    std::cerr << "Exception: " << e.what() << "\n";
  }
  return 0;
}
