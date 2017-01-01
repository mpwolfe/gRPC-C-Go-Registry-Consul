/*
 *
 * Copyright 2015, Google Inc.
 * All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions are
 * met:
 *
 *     * Redistributions of source code must retain the above copyright
 * notice, this list of conditions and the following disclaimer.
 *     * Redistributions in binary form must reproduce the above
 * copyright notice, this list of conditions and the following disclaimer
 * in the documentation and/or other materials provided with the
 * distribution.
 *     * Neither the name of Google Inc. nor the names of its
 * contributors may be used to endorse or promote products derived from
 * this software without specific prior written permission.
 *
 * THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS
 * "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT
 * LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR
 * A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT
 * OWNER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL,
 * SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT
 * LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE,
 * DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY
 * THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
 * (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE
 * OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
 *
 */

// **** MPW login_client.cc calls service:    Login - send User/Password to server
// helloworldmpw is the package name from proto file

#include <iostream>
#include <memory>
#include <string>

#include <grpc++/grpc++.h>
#include "customer.grpc.pb.h"

using grpc::ClientReader;
using grpc::ClientReaderWriter;
using grpc::ClientWriter;

using grpc::Channel;
using grpc::ClientContext;
using grpc::Status;


using customer::LoginRequest;
using customer::LoginReply;
using customer::Login;       // service

using customer::InfoRequest;
using customer::InfoReply;
using customer::Customer;       // service

using customer::detailRequest;
using customer::detailReply;
using customer::Customerdetail;          // service

// for registry to get address of microservice
using customer::RegisterRequest;
using customer::RegisterReply;
using customer::Register;          // service


using namespace std;

const string registryAddr ="127.0.0.1:50052";
const string serverAddr = "127.0.0.1:50051";
const string domain = "i52400deb";
const int port = 50051;

// call registry to find address of microservice
class RegisterClient {
 public:
  
  RegisterClient(std::shared_ptr<Channel> channel)
    : stub_(Register::NewStub(channel)) {}       // service is Register

  string registerservice(const string& cmd, const string& service, const string& domain, const string& address, int port) {

    cout <<"registerservice -> " << cmd << "\n";
    
    RegisterRequest request;
    request.set_cmd(cmd);
    request.set_service(service);
    request.set_domain(domain);
    request.set_address(address);
    request.set_port(port);

    // Container for the data we expect from the server.
    RegisterReply reply;
    ClientContext context;

    cout << "service " << service <<  " domain " << domain  << " address " << address << " port " << port << "\n";
     // The actual RPC - make actual RPC call on the stub..........
    Status status = stub_->RegisterService(&context, request, &reply);
    fprintf(stderr,",RegisterService status %s\n",status);
    cout << "register,  reply.message: reply.address: " << reply.message() << " " << reply.address() << "\n";
    
    return reply.address();
  }
  
private:
  std::unique_ptr<Register::Stub> stub_;
};

class LoginClient {
 public:
  LoginClient(std::shared_ptr<Channel> channel)
    : stub_(Login::NewStub(channel)) {}       // service is Login 

  // Assambles the client's payload, sends it and presents the response back
  // from the server.i RPC method
  
  string UserLogin(const string& user, string& password) {
    // Data we are sending to the server
    
    LoginRequest request;
    request.set_user(user);
    request.set_password(password);

    // Container for the data we expect from the server.
    LoginReply reply;

    // Context for the client. It could be used to convey extra information to
    // the server and/or tweak certain RPC behaviors.
    ClientContext context;

    // The actual RPC - make actual RPC call on the stub..........
    Status status = stub_->UserLogin(&context, request, &reply);

    // Act upon its status and reply.message
    if (status.ok()) {
      if (reply.message() == "invalid login") {
	return reply.message();
      } else {
	cout << reply.message() << "\n";     // Success
	return reply.message();
      }
    } else {
      std::cout << status.error_code() << ": " << status.error_message() << "\n";
      return "RPC failed for Login Service\n";;
    }  
  }

 private:
  std::unique_ptr<Login::Stub> stub_;
};

class CustomerClient {
 public:
  CustomerClient(std::shared_ptr<Channel> channel)
    : stub_(Customer::NewStub(channel)) {}       // service is Customer 

  string CustomerAccount(int  id) {
    // Data we are sending to the server

    InfoRequest request;
    request.set_custid(id);
    
// Container for the data we expect from the server.
    InfoReply reply;

    // Context for the client. It could be used to convey extra information to
    // the server and/or tweak certain RPC behaviors.
    ClientContext context;

    // The actual RPC - make actual RPC call on the stub..........
    Status status = stub_->CustomerInfo(&context, request, &reply);

    // Act upon its status and reply.message
    if (status.ok()) {
      if (reply.message() == "invalid account") {
	return reply.message();
      } else {
	cout << reply.message() << "\n";     // Success
	cout << reply.custid() << " " << reply.fname() << " " << reply.lname() << "\n";
	return reply.message();
      }
    } else {
      std::cout << status.error_code() << ": " << status.error_message() << "\n";
      return "RPC failed for Customer Service\n";;
    }  
  }
  private:
  std::unique_ptr<Customer::Stub> stub_;
};

class CustomerdetailClient {
 public : 
  CustomerdetailClient(std::shared_ptr<Channel> channel)
    : stub_(Customerdetail::NewStub(channel)) {}       // service is Customerdetail

  string getCustDetail(int custid) {
    printf("getCustDetail .....\n");
    
    detailRequest   request;               // data we are sending to server
    detailReply  reply;                        // data we expect from server
    ClientContext context;                // Context for client, convey extra info to the server
    
    detailReply detail;
    //request.set_id(id);
    request.set_custid(custid);
    
    // unique_ptr<ClientReader<detailReply>> reader;
    // The actual RPC - make actual RPC call on the stub..........
    // Status status = stub_->Custdetail(&context, request, &reply);
    //  auto status = stub_->Custdetail(&context, request, &reply);
    
    //    Status Custdetail(ServerContext* context, request, ServerWriter<detailReply>* writer) override {
    std::unique_ptr<ClientReader<detailReply>> reader(stub_-> Custdetail(&context, request ));
    //    auto status = stub_-> Custdetail(&context, request );
    
    printf("*** getCustDetail: before reader while loop. (records returned from server \n");
    while (reader->Read(&detail)) {
      cout << "custid " <<   detail.custid()
	   << ", "  << detail.fname() << ", " <<  detail.lname()
	   << ", "  << detail.product() << ", " <<  detail.price() << ", " << detail.desc() << "\n";
    }
    
    /*
    if (status.ok()) {
      return "RPC status OK";
    } else {
      return "RPC status Failed!";
    }
    return "OK";
    */
    return "OK";
  }
    
  private:
  std::unique_ptr<Customerdetail::Stub> stub_;
};

  /*
CustomerClient(std::shared_ptr<Channel> channel)
    : stub_(Customer::NewStub(channel)) {}       // service is Customer 
  */  
/* sqlite3
   .open users.db
   select * from users;
*/

// valid user/passwords: mwolfe/password1234, jsmith/password7777, bjones/password9999

int main(int argc, char** argv) {
  // Instantiate the client. It requires a channel, out of which the actual RPCs
  // are created. This channel models a connection to an endpoint (in this case,
  // localhost at port 50051). We indicate that the channel isn't authenticated
  // (use of InsecureChannelCredentials()).

  // Create a default SSL ChannelCredentials object
  // MPW  auto creds = grpc::SslCredentials(grpc::SslCredentialsOptions());
  // Create a channel using the credentials created in the previous step.
  // auto channel = grpc::CreateChannel(localhost:50051, creds);
  // MPW LoginClient login(grpc::CreateChannel("localhost:50051", creds) );
  
  // LoginClient login(grpc::CreateChannel("localhost:50051", grpc::InsecureChannelCredentials()));
  // LoginClient login(grpc::CreateChannel("99.32.248.90:50051", grpc::InsecureChannelCredentials()));
  // LoginClient login(grpc::CreateChannel("localhost:3000", grpc::InsecureChannelCredentials()));

  int id ;
  int custid;
  string user;
  string password;
  cout << argc << "\n";
  // authorized login
  if (argc == 4) {
    user = argv[1];
    password = argv[2];
    // id = atoi(argv[3]);
    custid = atoi(argv[3]);
  } else {
    printf("Usage  is : user, password, customer id\n");
    return 1;
  }

  // valid test data User: "mwolfe", password: "password1234", custid: 200

  string address = "";     // address of service returned from registry

  // call Registry, to get services address then call service
  RegisterClient  r1(grpc::CreateChannel(registryAddr, grpc::InsecureChannelCredentials()));
  address = r1.registerservice("GET","Login",domain,"",port); 
  cout << "**** return from GET " << address  << "\n";
  LoginClient login(grpc::CreateChannel(serverAddr, grpc::InsecureChannelCredentials()));
  string reply = login.UserLogin(user,password);  
  std::cout << "main: client received: " << reply << std::endl;
  if (reply == "invalid login") {
    return 0;
  }

  // call Registry, to get Customer service address, then call service
  address = r1.registerservice("GET","Customer",domain,"",port); 
  cout << "**** return from GET " << address  << "\n";
  CustomerClient  customer(grpc::CreateChannel(address, grpc::InsecureChannelCredentials()));
  reply = customer.CustomerAccount(custid);
  std::cout << "client received: " << reply << std::endl;
    
  // call Registry, to get Customerdetail service address then call service
  address = r1.registerservice("GET","Customerdetail",domain,"",port); 
  cout << "**** return from GET " << address  << "\n";
  CustomerdetailClient  custdetail(grpc::CreateChannel(address, grpc::InsecureChannelCredentials()));
  custdetail.getCustDetail(custid);

  return 0;
}
