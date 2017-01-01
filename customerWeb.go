package main

import (
	"net/http"
	"html/template"
	"strconv"
	"io"
	
	"google.golang.org/grpc/grpclog"
       "google.golang.org/grpc"
	pb "google.golang.org/grpc/examples/customer/customer"
	
	"goji.io"
	"goji.io/pat"
	"golang.org/x/net/context"
)

type Data struct {
	user string
	password string
}

// for /login GET
func login(ctx context.Context, w http.ResponseWriter, r *http.Request) {

	grpclog.Println("login handler")
	grpclog.Println("login: method:", r.Method) //get request method	
	render(w, "templates/login.html",nil)
}

/* **** start of login authorization and customer account services */
// Post -  /login, process user  login credentials
func processForm1(ctx context.Context, w http.ResponseWriter, r *http.Request) {
	grpclog.Println("processForm1: method: ", r.Method, r.RemoteAddr)

	r.ParseForm()   // parse the form
	user := r.FormValue("username")
	password := r.FormValue("password")
	grpclog.Printf("processForm1: user %v password %v\n",user,password)
	
    /* some test code, array of Data
	var d1 [10] Data
	d1[0].user = user
	d1[0].password = password
	grpclog.Printf("d1.user %v d1.password %v\n",d1[0].user, d1[0].password)	
*/
	data  := &Data{user,password}
	grpclog.Printf("data.user %v data.password %v\n",data.user, data.password)

	 //  *** this function will call RPC method !!
	if loginService(user,password) {   
		grpclog.Println("processForm1: login  success")
		render(w, "templates/account.html",nil)
	} else {
		grpclog.Println("processForm1: login failure")
		render(w, "templates/loginfailure.html",nil)
	}
}

// c pb.LoginClient, login Service
func loginService(user string, pass string) bool {
	grpclog.Println("loginService")

	// connect to Registry to obtain service address
        addr,result := connectRegistry("Login")
	if !result {
		grpclog.Fatal("RPC failure to Registry!")
		return false
	}
	
	// Setup a connection to the gRPC server.
        conn1, err := grpc.Dial(addr, grpc.WithInsecure())
        if err != nil {
                grpclog.Fatalf("did not connect: %v", err)
        }
	defer conn1.Close()
	c1 := pb.NewLoginClient(conn1)
	
	// RPC call -- to Login 
	r1, err := c1.UserLogin(context.Background(), &pb.LoginRequest{User: user, Password: pass})
	grpclog.Printf("loginService:  reply message -> %v\n",r1.Message)
	
	if err != nil {
                grpclog.Fatalf("RPC failure from server..... %v", err)
		return false
	}
	
	if r1.Message == "invalid login" {
		grpclog.Printf("Failure, client recieved -> %s", r1.Message)
		return false
	}
	
	grpclog.Printf("Success, client recieved -> %s", r1.Message)
	return true;
}

// Post -  account ID to display account info about a customer
func processForm2(ctx context.Context, w http.ResponseWriter, r *http.Request) {
	grpclog.Println("processForm2: method: ", r.Method, r.RemoteAddr)

	r.ParseForm()   // parse the form
	account := r.FormValue("account")
	id,_:=strconv.Atoi(account)        // converts to int
        id32 := int32(id)                        //  convert to int32
	grpclog.Printf("processForm2: accountId:  %d\n",id)
	
	reply  := CustomerService(id32)     //  *** this function will call RPC method !!
	
	//custid32 :=  int32(reply.Custid)

	if reply.Message == "invalid account" {
		grpclog.Printf("processForm2: Failure, client recieved -> %s", reply.Message)
	//	w.Header().Set("Content-Type", "application/json; charset=utf-8")
		w.Header().Set("Content-Type", "text/plain")
		w.Write([]byte("Account record not found!\n"))
		// infomsg := []string{"*Account not found!",}
		// a,_ := json.Marshal(infomsg)
		// w.Write(a)
		// render(w, "templates/account.html",nil)
		return
	}
	grpclog.Printf("processForm2: Account info: %d %s %s\n",reply.Custid,reply.Fname,reply.Lname)

	/*
        w.Header().Set("Content-Type", "application/json; charset=utf-8") 
        myItems := []string{"*Account Info*", "ID: " + account, "Name: " + reply.Fname  + " " + reply.Lname,
		"Email: " + reply.Email, "Phone: " + reply.Phone, "Street: " + reply.Street, "City: " + reply.City,
	"State: " + reply.State, "Zip: " + reply.Zip,}
        a, _ := json.Marshal(myItems)
        w.Write(a)
*/
	c1 := int(reply.Custid)
	custid := strconv.Itoa(c1)
	// mpw test code
	w.Header().Set("Content-Type", "text/plain")
	w.Write([]byte("* Account Info * \n"))
	// w.Write([]byte("ID:  "         + account + "\n"))
	w.Write([]byte("Custid  "   + custid + "\n"))
	w.Write([]byte("Name:   "  + reply.Fname + " " + reply.Lname +"\n"))
	w.Write([]byte("Email:  "    + reply.Email + "\n"))
	w.Write([]byte("Phone:  "   + reply.Phone + "\n"))
	w.Write([]byte("Street: "    + reply.Street + "\n"))
	w.Write([]byte("City:   "     + reply.City + "\n"))
	w.Write([]byte("State:  "    + reply.State + "\n"))
	w.Write([]byte("Zip:    "     +reply.Zip + "\n"))
	
}

// c pb.CustomerClient   Service
func  CustomerService(id int32) *pb.InfoReply {
	grpclog.Println("CustomerService")

	// connect to Registry to obtain service address
        addr,result := connectRegistry("Customer")
	if !result {
		grpclog.Fatal("RPC failure to Registry!")
	}
	
// Set up a connection to the gRPC server.
        conn, err := grpc.Dial(addr, grpc.WithInsecure())
        if err != nil {
                grpclog.Fatalf("did not connect: %v", err)
        }

	defer conn.Close()
	c := pb.NewCustomerClient(conn)

	// RPC call
	r, err := c.CustomerInfo(context.Background(), &pb.InfoRequest{Custid: id})
	grpclog.Printf("CustomerService:  reply message -> %v\n",r.Message)
	
	if err != nil {if err != nil {
                grpclog.Fatalf("RPC failure from server..... %v", err)
	}}

	
	if r.Message == "invalid account" {
		grpclog.Printf("CustomerService: Failure, client recieved -> %s", r.Message)
	} else {
		grpclog.Printf("CustomerService: Success, client recieved -> %s", r.Message)
	}
	return r
}

// Post - get Customer ID to display customer purchases from custdetail database
func processForm3(ctx context.Context, w http.ResponseWriter, r *http.Request) {
	grpclog.Println("processForm3: method: ", r.Method, r.RemoteAddr)

	r.ParseForm()   // parse the form
	custid := r.FormValue("customer")
	id,_:=strconv.Atoi(custid)           // converts to int
        id32 := int32(id)                        //  convert to int32
	grpclog.Printf("processForm3: Customer Id:  %d\n",id)
	
	CustdetailService(id32,w)
	grpclog.Printf("processForm3:  after call to CustdetailService")
}

// c pb.CustomerdetailClient   Service
// func  CustdetailService(custid int32) *pb.InfoReply {
func  CustdetailService(custid int32, w http.ResponseWriter) {
	grpclog.Println("CustdetailService")

	// connect to Registry to obtain service address
        addr,result := connectRegistry("Customerdetail")
	if !result {
		grpclog.Fatal("RPC failure to Registry!")
	}
	
// Set up a connection to the gRPC server.
        conn, err := grpc.Dial(addr, grpc.WithInsecure())
        if err != nil {
                grpclog.Fatalf("did not connect: %v", err)
        }

	defer conn.Close()   // close connection to server
	c := pb.NewCustomerdetailClient(conn)
	// RPC call
	stream, err := c.Custdetail(context.Background(), &pb.DetailRequest{Custid: custid})
	if err != nil {
		grpclog.Fatalf("CustdetailService: RPC failure from server..... %v", err)
                // grpcgrpclog.Fatalf("%v.ListFeatures(_) = _, %v", client, err)
        }
	grpclog.Println("sucess, calling RPC Customerdetail")

	w.Header().Set("Content-Type", "text/plain")
	w.Write([]byte("* Customer Detail * \n"))

	var count = 0
	for {
		detail, err := stream.Recv()
		if err == io.EOF && count == 0{
			w.Write([]byte("* Customer Detail Records not found!* \n"))
			break
		}
		if err == io.EOF {
			break
		}
		if err != nil {
			grpclog.Fatalf("CustdetailService: RPC failure from server, detail recs %v", err)
		}
		
		// write detail records to page
		grpclog.Println("**** detail -> ",detail)

		p1 := float64(detail.Price)
		price := strconv.FormatFloat(p1, 'f', 2, 32)     // float to string
		// id := strconv.Itoa(int (detail.Id))                     // int to string
		custid := strconv.Itoa(int (detail.Custid))        // int to string
		// w.Write([]byte("ID:      "      + id + "\n"))
		w.Write([]byte("CustID   "   + custid + "\n"))
		w.Write([]byte("Name:    "  + detail.Fname + " " + detail.Lname +"\n"))
		w.Write([]byte("Product  "  + detail.Product + "\n"))
		w.Write([]byte("Price    "    + price + "\n"))
		w.Write([]byte("Desc     "   + detail.Desc + "\n\n"))
		count++
	}
	
	recs := strconv.Itoa(int (count))    // int to string
	w.Write([]byte("* Number of Customer Records:    "   + recs + "\n"))
	grpclog.Println(" *** Number of Customer Detail records ", count)	
}

func render(w  http.ResponseWriter, file string, data interface{}) {
	grpclog.Printf("render template -> %s\n",file)

	tmpl, err := template.ParseFiles(file)
	if err != nil {
		http.Error(w, err.Error(), http.StatusInternalServerError)
	}

	if err := tmpl.Execute(w,data); err != nil {
		http.Error(w, err.Error(), http.StatusInternalServerError)
	}
}

// currently not used....
func confirmation(ctx context.Context, w http.ResponseWriter, r *http.Request) {
	grpclog.Println("handler -> confirmation")
	render(w, "templates/confirmation.html",nil)
}

const (
	registryaddress = "127.0.0.1:50052"
	serverAddr = "127.0.0.1:8000"
	defaultUser = "mwolfe"
	domain = "i52400deb"
	defaultPassword = "password1234"
)

// Connect to Registry and return address of service
func connectRegistry(service string) (string,bool) {
	grpclog.Println("connectRegistry")

        result := false
	
// setup a connnect to the Registry server
	conn, err := grpc.Dial(registryaddress, grpc.WithInsecure())
        if err != nil {
                grpclog.Fatalf("did not connect to Registry: %v", err)
		result = false
        }
	
	defer conn.Close()
	c := pb.NewRegisterClient(conn)
	
	// RPC call 
	r, err := c.RegisterService(context.Background(),
		&pb.RegisterRequest{Cmd: "GET", Service: service, Domain: domain})

	if err != nil {
                grpclog.Fatalf("RPC failure from server..... %v", err)
		result = false
	}
	if r.Message == "failure" {
		grpclog.Printf("Failure, client recieved -> %s", r.Message)
		result = false
	}
	
	result = true
	grpclog.Printf("service address -> \n",r.Address)
	return r.Address,result
}

func main() {
	
	mux := goji.NewMux()
	mux.HandleFuncC(pat.Get("/login"),login)                      
	mux.HandleFuncC(pat.Post("/login"),processForm1)
	mux.HandleFuncC(pat.Post("/account"),processForm2)
	mux.HandleFuncC(pat.Post("/customer"),processForm3)
	grpclog.Println("customerWeb:   listening on port 8000")
	http.ListenAndServe(serverAddr, mux)
}
