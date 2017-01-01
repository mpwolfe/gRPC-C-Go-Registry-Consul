# gRPC-with-simple-Registry

This project is a simple demonstration that uses gRPC (Server and Client), gRPC Registry
and consul by HashiCorp as a backend Key/Value datastore.

The main ideas are as follows:

    - The server registers its services with the Registry
    - The Registry uses the consul REST api to store key/value pair of services and ip address
    - The client makes an RPC call to the registry to get the ip address of a service
    - When the server is terminates (CTRL-C) it will call the Registry to deregister its services
    - The registry will call consul via the REST api to delete the services

    server -> registry -> consul (to register service via a PUT)
    client -> registry  -> conusl (to lookup the service via a GET)
    server -> registry -> conusl (to deregister a service via a DELETE)

To demonstrate the above ideas I using the following services:

These services could be some initial services for a Telcom company (ATT) to
view customer info and products that have been purchased.

   - Login:  for user login with username and password
   - Customer:  the user  gets infomation about a customer
   - Customerdetail:  allows the user to get customer products that the customer has purchased
   -  Register:  for server to register and deregister services and client to query it for servie ip addresses 

I am using SQLITE3 to store login,customer and customerdetail data

The programs are as follows:

    - customerServer.cpp the gRPC server in c++
    - customerClient.cpp  the command line gRPC client written in c++
    - consul-registry.go    the Registry for gRPC client/server and REST api calls to consul by Hashicorp in go
    - customerWeb.go     the gRPC web based client written in go

The go programs are using additional software:

    google.golang.org/grpc/grpclog - for logging
    google.golang.org/grpc              - gRPC for go
    goji.io, goji.io/pat                        - for web interface
    consulapi  "github.com/hashicorp/consul/api   - consul REST api for the registry

The c++ programss are using additional software:

    SQLITE3 as a shared library with embedded c/c++ api
    protocol buffers version 3
    gRPC for c++

To get a feel for how the software fits together we do the following.
Note: all clients and servers are configured for localhost 127.0.0.1

1. start the consul software -> consul agent   -dev -client=127.0.0.1
2. start the registry -> consul-registry
3. start the server -> customerServer
4. start the client  ->  customerClient mwolfe password1234 200   # username password and customer id
5. start the go client web program -> customerWeb # http://127.0.0.1:8000/login 
   Username mwolfe
   Password password1234

   Account Id 200
   Customer ID 200
  
6. CTRL-C the customerServer which will deregister the services

Note: the key for the registry is: service + "/" + domain     ex: Login/i52400deb
         the value is:                     address + ":" + port         ex: 127.0.0.1:50051



    



