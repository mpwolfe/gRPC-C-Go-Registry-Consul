#!/bin/bash
protoc --go_out=plugins=grpc:. *.proto
mv  customer.pb.go customer/.
