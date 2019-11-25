%% ====================================================================
%% @Author:	Payton
%% @Date:	2019-09-04 08:31:20
%% @Doc:	DESC
%% @Last:	2019-11-25 21:26:59
%% ====================================================================

-module(server).

%% ====================================================================
%% API functions
%% ====================================================================
-export([
		main/0
	]).

%% ====================================================================
%% Internal functions
%% ====================================================================

main() ->
	case gen_tcp:listen(6000, [binary, {packet, 0}, {active, false}]) of
		{ok, ListenSocket} ->
			loop_accept(ListenSocket);
		Reason ->
			io:format("socket_listen_fail:~p~n",[Reason]),
			ListenSocket = undefined,
			throw(Reason)
	end,
	timer:sleep(60000),
	gen_tcp:close(ListenSocket),
	ok.

loop_accept(ListenSocket) ->
	case gen_tcp:accept(ListenSocket) of
		{ok, Socket} ->
			spawn(fun() -> handle_socket(Socket) end);
		Reason ->
			io:format("socket_accept_fail:~p~n",[Reason]),
			loop_accept(ListenSocket)
	end.

handle_socket(Socket) ->
	case gen_tcp:recv(Socket,0) of
		 {ok, Packet} ->
		 	io:format("recv size:~p~n",[byte_size(Packet)]),
		 	gen_tcp:send(Socket,Packet);
		 {error, closed} ->
		 	exit(normal);
		 Reason ->
		 	io:format("socket_recv_fail:~p~n",[Reason])
	end,
	handle_socket(Socket).