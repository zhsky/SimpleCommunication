%% ====================================================================
%% @Author:	Payton
%% @Date:	2019-09-04 18:31:20
%% @Doc:	DESC
%% @Last:	2019-11-26 17:53:44
%% ====================================================================

-module(client).

%% ====================================================================
%% API functions
%% ====================================================================
-export([
		main/1
		,send_recv/1
		,socket_connect/1
		,send_data/1
		,send_big_data/1
	]).

%% ====================================================================
%% Internal functions
%% ====================================================================

main(_Num) ->
	socket_connect(_Num).

send_recv(_Num) ->
	{ok, Socket} = gen_tcp:connect({192,168,16,240},6000,[binary,{active, false}, {packet, 0}]),
	Fun = fun(Loop) ->
		case gen_tcp:recv(Socket,0) of
			 {ok, Packet} -> 
			 	io:format("recv size:~p~n",[byte_size(Packet)]);
			 {error, closed} ->
			 	exit(normal);
			 {error, Reason} ->
			 	io:format("recv error:~p~n",[Reason])
		end,
		Loop(Loop)
	end,
	spawn(fun() ->
		Fun(Fun)
	end),
	lists:foreach(fun(_) ->
		send_data(Socket)
	end,lists:seq(1,10)),
	timer:sleep(5000),
	gen_tcp:close(Socket).

send_big_data(_Num) ->
	SendBytes = 10 * 1024 * 1024,
	Bin = list_to_binary([1 || _ <- lists:seq(1,SendBytes)]),

	{ok, Socket} = gen_tcp:connect({192,168,16,240},6000,[binary,{active, false},{packet, 0}]),

	Id = 16#ffffff,
	Name = list_to_binary("Jack"),
	NameSize = byte_size(Name),
	Content = Bin,
	ContentLen = byte_size(Content),

	Data = <<Id:64/big,NameSize:16/big,Name/binary,ContentLen:16/big,Content/binary>>,
	DataLen = byte_size(Data),
	Msg = <<DataLen:32/big,Data/binary>>,
	gen_tcp:send(Socket,Msg),

	file:write_file("./send_file.txt",Data),
	io:format("Id,NameSize,Name,ContentLen,DataLen ~p~n",[{Id,NameSize,ContentLen,DataLen}]),
	receive after 5000 -> skip end,
	gen_tcp:close(Socket).

socket_connect(Num) ->
	statistics(wall_clock),
	Supid = self(),
	Fun = fun() ->
		case catch gen_tcp:connect({192,168,16,240},6000,[binary,{active, false}, {packet, 0}],infinity) of
			{ok, Socket} -> 
				send_data(Socket),
				gen_tcp:close(Socket),
				Supid ! finish;
			Reason ->
			 	io:format("connect error:~p~n",[Reason]),
				Supid ! false,
				skip
		end
	end,
	[spawn(Fun) || _ <- lists:seq(1,Num)],

	Ret = loop_wait(0,Num),
	io:format("false:~p~n",[Ret]),
	{_,Time} = statistics(wall_clock),
	io:format("~p~n",[Time]).

loop_wait(Failed,0) -> Failed;
loop_wait(Failed,N) ->
	receive
		finish ->
			loop_wait(Failed, N - 1);
		false ->
			loop_wait(Failed + 1,N - 1)
	end.

send_data(Socket) ->
	SendBytes = 1 * 1024,
	Bin = list_to_binary([1 || _ <- lists:seq(1,SendBytes)]),

	Id = 16#ffffff,
	Name = list_to_binary("Jack"),
	NameSize = byte_size(Name),
	Content = Bin,
	ContentLen = byte_size(Content),

	Data = <<Id:64/big,NameSize:16/big,Name/binary,ContentLen:16/big,Content/binary>>,
	DataLen = byte_size(Data),
	Msg = <<DataLen:32/big,Data/binary>>,
	gen_tcp:send(Socket,Msg),
	ok.