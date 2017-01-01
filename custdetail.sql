create table custdetail(
	cust_id int ,
	product varchar(30),
	price real,
	desc  varchar(50)
);

create index cust_id_idx on custdetail(cust_id);
