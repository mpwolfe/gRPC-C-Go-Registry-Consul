package main

import (
//	"os"
//	"fmt"
//	"time"
//	"log"
	"net"
	// "strings"
        "strconv"

	consulapi  "github.com/hashicorp/consul/api"

	"golang.org/x/net/context"
        "google.golang.org/grpc"

	"google.golang.org/grpc/grpclog"
	// "github.com/golang/protobuf/proto"
	
        pb "google.golang.org/grpc/examples/customer/customer"

)

/*
type KVPair struct {
    Key         string
    CreateIndex uint64
    ModifyIndex uint64
    LockIndex   uint64
    Flags       uint64
    Value       []byte
    Session     string
}
*/

type server struct{}

// Service Register
func (s *server) RegisterService(ctx context.Context, in *pb.RegisterRequest) (*pb.RegisterReply,error) {

	grpclog.Println("RPC -> RegisterService")
	grpclog.Println("cmd, service, domain, address, port  ", in.Cmd, in.Service, in.Domain, in.Address, in.Port)

	x := int(in.Port)
	strPort := strconv.Itoa(x)
	err := ""
	value := ""
	
	switch in.Cmd {
	case "PUT" :
		grpclog.Println("processing PUT")
		err = register(in.Service, in.Domain, in.Address, strPort)
	case "GET" :
		grpclog.Println("processing GET")
		err,value = retrieve(in.Service,in.Domain)
		grpclog.Println("** return from retrieve(GET) ",err,value)
		return &pb.RegisterReply{Message: err, Address: value},nil
	case "DELETE" :
		grpclog.Println("processing DELETE")
		err = deregister(in.Service,in.Domain)
	default:
		grpclog.Println("error, invalid command")
		return &pb.RegisterReply{Message: "failure"},nil
	}
		
	// result = register(in.Service, in.Domain, in.Address, strPort)
	grpclog.Println("RegisterService, result -> ",err)
	return &pb.RegisterReply{Message: err},nil
}

/*
// service Deregister
func (s *server) DeregisterService(ctx context.Context, in *pb.DeregisterRequest) (*pb.DeregisterReply,error) {

	grpclog.Println("RPC -> DeregisterService")
	grpclog.Println("service, domain: ", in.Service, in.Domain)
	
	result := deregister(in.Service,in.Domain)
	grpclog.Println("DeregisterService, result -> ",result)
	
	return &pb.DeregisterReply{Message: "Success"},nil
}

// service Retrieve
func (s *server) RetrieveService(ctx context.Context, in *pb.RetrieveRequest) (*pb.RetrieveReply,error) {

	grpclog.Println("RPC -> RetrieveService")
	grpclog.Println("service, domain: ", in.Service, in.Domain)
	
	result := retrieve(in.Service,in.Domain)
	grpclog.Println("RetrieveService, result -> ",result)
	
	return &pb.RetrieveReply{Message: "Success"},nil
}
*/

// Put
func register( service string, domain string, address string, port string)  (string) {

	grpclog.Println("register ",service,domain,address,port )
	
	// configure a client for consul api
	config := consulapi.DefaultConfig()
	config.Address = consuladdr
	consul, err := consulapi.NewClient(config)
	if err != nil {
		grpclog.Println("register(1) ", err)
		return "failure"
	}

	id := consul.Session()
	session,_,err := id.Create(nil,nil)
	if err != nil {
		grpclog.Println("register(2) -> error Create session ",err)
		return "failure"
	}
	grpclog.Println("session id -> ",session)
	kv := consul.KV()

	key := service + "/" + domain
	value := address + ":" + port
	grpclog.Println("register: key, value",key,value)
	
	// put
	// d := &consulapi.KVPair{Key: "login/i52400deb", Value: []byte("99.32.248.90:50001"),Session: session}
	d := &consulapi.KVPair{Key: key, Value:  []byte(value), Session: session}
	kv.Acquire(d, nil)

	return "success"
}

// Get
func retrieve( service string, domain string) (string,string) {

	grpclog.Println("retrieve(Get) ", service,domain)
	
	// configure a client for consul api
	config := consulapi.DefaultConfig()
	config.Address = consuladdr
	consul, err := consulapi.NewClient(config)
	if err != nil {
		grpclog.Println("retrieve(1) ", err)
		return "failure",""
	}

	id := consul.Session()
	session,_,err := id.Create(nil,nil)
	if err != nil {
		grpclog.Println("retrieve -> error Create session ",err)
		return "failure",""
	}
	grpclog.Println("session id -> ",session)
	kv := consul.KV()

	key := service + "/" + domain
	// Get
	resp,qm,err := kv.Get(key, nil)
	
	if err != nil {
		grpclog.Println( "retrieve(2) ",err)
		return "failure",""
	}
	if resp == nil {
		grpclog.Println("resp == nil ",qm)
		return "failure",""
	}
	
	grpclog.Println("Value of Get (Session)  -> ", string(resp.Session))
	grpclog.Println("Value of Get (key)  -> ", string(resp.Value))
	
	return "success",string(resp.Value)
}

// Delete
func deregister( service string, domain string) (string) {
	
	grpclog.Println("Deregister(Delete)", service,domain)

	// configure a client for consul api
	config := consulapi.DefaultConfig()
	config.Address = consuladdr
	consul, err := consulapi.NewClient(config)
	if err != nil {
		grpclog.Println("deregister(1) ", err)
		return "failure"
	}

	key := service + "/" + domain
	grpclog.Println("Deregistering key -> ",key)
	
	id := consul.Session()
	session,_,err := id.Create(nil,nil)
	if err != nil {
		grpclog.Println("deregister(2) error Create session ",err)
		return "failure"
	}
	grpclog.Println("session id -> ",session)
	kv := consul.KV()

	// Delete
	grpclog.Println("cmd -> delete")
	_,err = kv.Delete(key,nil)
	
	if err != nil {
		grpclog.Println("Deregister(3) delete ",err)
		return "failure"
	}
	return "success"
}


const (
        port = ":50052"
	consuladdr = "127.0.0.1:8500"
)

func main() {

//	register("login","i52400deb","99.32.248.90","50051")
//	retrieve("login","i52400deb")
//	deregister("login","i52400deb")

	// ************************************************************
        grpclog.Println("consul-registry is starting......")
	
	lis, err := net.Listen("tcp", port)
	if err != nil {
		grpclog.Fatalf("consul-registry, failed to listen: %v", err)
	}
	
	s := grpc.NewServer()
	// Note: pb.RegisterNameofserverServer(s,&server{})	
	pb.RegisterRegisterServer(s, &server{})
	grpclog.Println("Registered server......")
	
	if err := s.Serve(lis); err != nil {
		grpclog.Fatalf("consul-registry: failed to serve: %v", err)
	}
}

