// customerServer.cpp


#include <iostream>
#include <memory>
#include <string>
#include <cerrno>     // throw string(strerror(errno));
#include <cstring>
#include <cstdlib>
#include <cstdio>
#include <vector>
#include <csignal>   // to deregister service on CTRL-C

#include <sqlite3.h>   // for sqlite

#include "customer.grpc.pb.h"
#include <grpc/grpc.h>
#include <grpc++/grpc++.h>    // as ac client

#include <grpc++/server.h>
#include <grpc++/server_builder.h>
#include <grpc++/server_context.h>
#include <grpc++/security/server_credentials.h>

using grpc::Server;
using grpc::ServerBuilder;
using grpc::ServerContext;
using grpc::Status;

using grpc::ServerReader;
using grpc::ServerReaderWriter;
using grpc::ServerWriter;

// for server acting as client calling Registry server
using grpc::Channel;
using grpc::ClientContext;
using grpc::ClientReader;
using grpc::ClientReaderWriter;
using grpc::ClientWriter;
// **************************************************

// MPW
using customer::LoginRequest;
using customer::LoginReply;
using customer::Login;                  // service

using customer::InfoRequest;
using customer::InfoReply;
using customer::Customer;          // service

using customer::detailRequest;
using customer::detailReply;
using customer::Customerdetail;   // service


// for registry to get address of microservice
using customer::RegisterRequest;
using customer::RegisterReply;
using customer::Register;          // service

using namespace std;


const string s1 = "Login";
const string s2 = "Customer";
const string s3 = "Customerdetail";

const string registryAddr ="127.0.0.1:50052";
const string address = "127.0.0.1";  
const string serverAddr = "127.0.0.1:50051";
const int port = 50051;
const string domain = "i52400deb";
  
// server now as client calling Regsitry server!!
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


// Logic and data behind the server's behavior, note: Login is the service in proto file
// for Login service
class LoginServiceImpl final : public Login::Service {

  // RPC - in proto: rpc UserLogin(LoginRequest) returns (LoginReplay)
  Status UserLogin(ServerContext* context, const LoginRequest* request, LoginReply* reply) override {
    
    sqlite3 *db;
    int rc;
    sqlite3_stmt *res;

    rc = sqlite3_open("users.db",&db);
    if (rc != SQLITE_OK) {
      fprintf(stderr, "can't open database %s\n",sqlite3_errmsg(db));
      exit(1);
    }

    const char *user =  request->user().c_str();
    const char *password = request->password().c_str();
    
    //string sql ("select exists (select *  from users where user  = ? and password = ? )" );
    //string sql ("select *  from users where user  = ?" );
    
    string sql ("select *  from users where user  = ? and password = ?" );
    const char *sqlc = sql.c_str();
    rc = sqlite3_prepare_v2(db,sqlc, strlen(sqlc) + 1, &res, 0);
    
    if (rc == SQLITE_OK) {
      fprintf(stderr, " sql prepared statement looks good [%s]\n",sqlc);
      fprintf(stderr, " binding user -> %s  password -> %s\n",user,password);
      sqlite3_bind_text(res,1,user,strlen(user),0);
      sqlite3_bind_text(res,2,password,strlen(password),0);
    }
    else {
      fprintf(stderr, "Failed to execute statement: %s\n", sqlite3_errmsg(db));
    }

    int step = sqlite3_step(res);
    fprintf(stderr, "step value -> %d\n",step);
    fprintf(stderr, " colume(0) -> %s: ", sqlite3_column_text(res, 0));
    fprintf(stderr, " colume(1) -> %s: ", sqlite3_column_text(res, 1));
    fprintf(stderr, " colume(2) -> %s: \n", sqlite3_column_text(res, 2));
    if (step == SQLITE_ROW) {
      fprintf(stderr, "successful user login\n");
      reply->set_message(request->user() + ": successful login");
    }
    else {
      fprintf(stderr, "invalid login!\n");
      //      reply->set_message(request->user() + " invalid login");
      reply->set_message("invalid login");
    }
    
    // 100 = SQLITE_ROW indicates another row of output available
    // 101 = SQLITE_DONE indicates that an operation has completed, statement run to completion

    sqlite3_finalize(res);
    sqlite3_close(db);
    
    return Status::OK;
  }
};

// Service for Customer
class CustomerServiceImpl final : public Customer::Service {

  // rpc call
  Status CustomerInfo(ServerContext* context, const InfoRequest* request, InfoReply* reply) override {

    sqlite3 *db;
    int rc;
    sqlite3_stmt *res;

    rc = sqlite3_open("customers.db",&db);
    if (rc != SQLITE_OK) {
      fprintf(stderr, "can't open database %s\n",sqlite3_errmsg(db));
      exit(0);
    }

    const unsigned int id =  request->custid();
    string sql ("select *  from customer where cust_id  = ? " );
    const char *sqlc = sql.c_str();
    rc = sqlite3_prepare_v2(db,sqlc, strlen(sqlc) + 1, &res, 0);
    
    if (rc == SQLITE_OK) {
      fprintf(stderr, " sql prepared statement looks good [%s]\n",sqlc);
      fprintf(stderr, " binding id-> %d\n",id);
      sqlite3_bind_int(res,1,id);
    }
    else {
      fprintf(stderr, "Failed to execute statement: %s\n", sqlite3_errmsg(db));
    }

    int step = sqlite3_step(res);
    fprintf(stderr, "step value -> %d\n",step);
    // fprintf(stderr, " %d: ", sqlite3_column_int(res, 0));
    fprintf(stderr, " %d: ", sqlite3_column_int(res, 0));
    fprintf(stderr, " %s: ", sqlite3_column_text(res, 1));
    fprintf(stderr, " %s: ", sqlite3_column_text(res, 2));
    fprintf(stderr, " %s: ", sqlite3_column_text(res, 3));
    fprintf(stderr, " %s: ", sqlite3_column_text(res, 4));
    fprintf(stderr, " %s: ", sqlite3_column_text(res, 5));
    fprintf(stderr, " %s: ", sqlite3_column_text(res, 6));
    fprintf(stderr, " %s: ", sqlite3_column_text(res, 7));
    fprintf(stderr, " %s: \n", sqlite3_column_text(res, 8));
    
    if (step == SQLITE_ROW) {
      fprintf(stderr, "successful, customer account !  \n");
      reply->set_message("successful  customer account");
      // reply->set_id(sqlite3_column_int(res,0));
      reply->set_custid(sqlite3_column_int(res,0));
      reply->set_fname(reinterpret_cast<const char*>(sqlite3_column_text(res,1)) );
      reply->set_lname(reinterpret_cast<const char*>(sqlite3_column_text(res,2)) );
      reply->set_email(reinterpret_cast<const char*>(sqlite3_column_text(res,3)) );
      reply->set_phone(reinterpret_cast<const char*>(sqlite3_column_text(res,4)) );
      reply->set_street(reinterpret_cast<const char*>(sqlite3_column_text(res,5)) );
      reply->set_city(reinterpret_cast<const char*>(sqlite3_column_text(res,6)) );
      reply->set_state(reinterpret_cast<const char*>(sqlite3_column_text(res,7)) );
      reply->set_zip(reinterpret_cast<const char*>(sqlite3_column_text(res,8)) );
      cout << " Cust ***  id: " <<  reply->custid()  << " first name: " << reply->fname() << "\n";
    }
    else {
      fprintf(stderr, "invalid account \n");
      reply->set_message("invalid account");
    }
    
    sqlite3_finalize(res);
    sqlite3_close(db);
    
    return Status::OK;
  }
};


// service for Customerdetail
class CustomerdetailServiceImpl final : public Customerdetail::Service {

  // stream reply to client.....
  Status Custdetail(ServerContext* context, const detailRequest* request, ServerWriter<detailReply>* writer) override {
    fprintf(stderr," **** Custdetail RPC\n");
    //fprintf(stderr,"***  Custdetail: request->  id: %d custid: %d\n",request->id(), request->custid());
    fprintf(stderr,"***  Custdetail: request->  custid: %d\n", request->custid());
    
    detailReply reply;
    sqlite3 *db;
    int rc;
    sqlite3_stmt *res;
    
     rc = sqlite3_open("customers.db",&db);
    if (rc != SQLITE_OK) {
      fprintf(stderr, "can't open database %s\n",sqlite3_errmsg(db));
      exit(0);
    }

    // const unsigned int id =  request->id();
    const unsigned int custid = request->custid();

    string sql ("select customer.cust_id,fname, lname, product,price,desc from customer"
		" inner join custdetail on customer.cust_id = custdetail.cust_id and customer.cust_id = ?");
    
    const char *sqlc = sql.c_str();
    rc = sqlite3_prepare_v2(db,sqlc, strlen(sqlc) + 1, &res, 0);
    
    if (rc == SQLITE_OK) {
      sqlite3_bind_int(res,1,custid);              // *** bind value ?
      fprintf(stderr, " sql prepared statement looks good [%s]\n",sqlc);
      fprintf(stderr, " binding custid-> %d\n",custid);
    }
    else {
      fprintf(stderr, "Failed to execute statement: %s\n", sqlite3_errmsg(db));
    }

    int count = 0;
    int step;
    // int step = sqlite3_step(res);
    while (sqlite3_step(res) == SQLITE_ROW) {      
      fprintf(stderr, "successful, customer -> custdetail records.....  \n");
      // reply.set_id(sqlite3_column_int(res,0));
      reply.set_custid(sqlite3_column_int(res,0));
      reply.set_fname(reinterpret_cast<const char*>(sqlite3_column_text(res,1)) );
      reply.set_lname(reinterpret_cast<const char*>(sqlite3_column_text(res,2)) );
      reply.set_product(reinterpret_cast<const char*>(sqlite3_column_text(res,3)) );
      reply.set_price(sqlite3_column_double(res,4));
      reply.set_desc(reinterpret_cast<const char*>(sqlite3_column_text(res,5)) );
      fprintf(stderr, "step value -> %d\n",step);
      // fprintf(stderr, " %d: ", sqlite3_column_int(res, 0));
      fprintf(stderr, " %d: ", sqlite3_column_int(res, 0));
      fprintf(stderr, " %s: ", sqlite3_column_text(res, 1));
      fprintf(stderr, " %s: ", sqlite3_column_text(res, 2));
      fprintf(stderr, " %s: ", sqlite3_column_text(res, 3));
      fprintf(stderr, " %f: ", sqlite3_column_double(res, 4));
      fprintf(stderr, " %s: \n", sqlite3_column_text(res, 5));
      writer->Write(reply);
      count++;
    }
    fprintf(stderr,"Total found customer detail records: %d\n",count);  

    sqlite3_finalize(res);
    sqlite3_close(db);
  
    return Status::OK;
  }

 private:
  vector<Customer> cust_list;
};


/* ****  start the server */
void RunServer() {

  fprintf(stderr,"RunServer.......\n");

  LoginServiceImpl service;                     // service
  CustomerServiceImpl service1;             //  service
  CustomerdetailServiceImpl service2;     //  service
  
  ServerBuilder builder;
  builder.AddListeningPort(serverAddr, grpc::InsecureServerCredentials());
  builder.RegisterService(&service);
  builder.RegisterService(&service1);
  builder.RegisterService(&service2);

  std::unique_ptr<Server> server(builder.BuildAndStart());
  cout << "Server listening on " << serverAddr << std::endl;
  server->Wait();
}

// CTRL-C, then deregister service at grpc registry service
void signal_handler(int signal) {
  fprintf(stderr,"signal_handler %d\n",signal);
  fprintf(stderr,"Deregistering service at registry.........\n");

  string result = "";
  
  // services: Login,Customer,Customerdetail
  // Registry
  RegisterClient  r1(grpc::CreateChannel(registryAddr, grpc::InsecureChannelCredentials()));

  // Server calls Deregistry via gRPC to register services(s1,s2,s3) with service/domain as the Key
  result = r1.registerservice("DELETE",s1,domain,"",port);
  cout << "**** return from DELETE " << result  << "\n";
  
  result = r1.registerservice("DELETE",s2,domain,"",port);
  cout << "**** return from DELETE " << result  << "\n";

  result = r1.registerservice("DELETE",s3,domain,"",port);
  cout << "**** return from DELETE " << result  << "\n";
  
  exit(0);
}


/* ************************************ */
int main(int argc, char** argv) {

  // Install a signal handler
  std::signal(SIGINT, signal_handler);
  
  fprintf(stderr,"start of  customerServer........\n");

  string result = "";

  // services: Login,Customer,Customerdetail
  // Registry
  RegisterClient  r1(grpc::CreateChannel(registryAddr, grpc::InsecureChannelCredentials()));

  // Server calls Registry via gRPC to register services(s1,s2,s3) at service/doman  as Key
  result = r1.registerservice("PUT",s1,domain,address,port);
  cout << "**** return from PUT " << result  << "\n";

  result = r1.registerservice("PUT",s2,domain,address,port);
  cout << "**** return from PUT " << result  << "\n";

  result = r1.registerservice("PUT",s3,domain,address,port);
  cout << "**** return from PUT " << result  << "\n";

  // now run the server!
  RunServer();

  return 0;
}


/*
while (sqlite3_step(res) == SQLITE_ROW) {
    34                  printf("%s|", sqlite3_column_text(res, 0));
    35                  printf("%s|", sqlite3_column_text(res, 1));
    36                  printf("%s|", sqlite3_column_text(res, 2));
    37                  printf("%u\n", sqlite3_column_int(res, 3));
    38
    39                  rec_count++;
    40          }
*/

  /*
    sqlite3_finalize(res);
    sqlite3_close(db);
    // reply.set_id(request->id());
    reply.set_custid(request->custid());
    reply.set_fname("michael");
    reply.set_lname("wolfe");
    writer->Write(reply);

    reply.set_id(101);
    reply.set_custid(301);
    reply.set_fname("jim");
    reply.set_lname("smith");
    writer->Write(reply);
    */

/*
select id, customer.cust_id, lname, product,price,desc from customer inner  join custdetail
 on customer.cust_id = custdetail.cust_id and customer.id = 103;
*/

/*
select services.service, address,port from services inner join servicesdetail
 on services.service = servicesdetail.service and services.service = "Customer";

*/
