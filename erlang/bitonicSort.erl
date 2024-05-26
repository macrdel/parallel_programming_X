-module(bitonicSort).
-export([main/0, main/3]).

% генерация массива, запуск сортировки, замер времени и вывод результата.
main() ->
    main(4, 4, true).

main(NSize, NumProcs, true) ->
    Array = generate_array(NSize),
    io:format("~p => ", [Array]),
    {Time, SortedArray} = timer:tc(fun() -> bitonic_sort(Array, NumProcs) end),
    io:format("~p~n", [SortedArray]),
    io:format("~p microseconds~n", [Time]);

main(NSize, NumProcs, false) ->
    Array = generate_array(NSize),
    {Time, _} = timer:tc(fun() -> bitonic_sort(Array, NumProcs) end),
    io:format("~p microseconds~n", [Time]).

bitonic_connect(Xs, NumProcs) ->
  L = length(Xs) div NumProcs,
  if
    L =< 2 ->
      bitonic_connect_sequential(Xs);
    true ->
      {Left, Right} = lists:split(L, Xs),
      Mins = [min(A,B) || {A,B} <- lists:zip(Left, Right)],
      Maxs = [max(A,B) || {A,B} <- lists:zip(Left, Right)],
      Parent = self(),
      R = make_ref(),
      spawn_link(fun() -> Result = bitonic_connect(Maxs, NumProcs), Parent ! {R, Result} end), 
      Minsconnected = bitonic_connect(Mins, NumProcs),
      Maxsconnected = receive
        {R, Result} -> Result
    end,
    lists:append (Minsconnected, Maxsconnected)
end.

bitonic_connect_sequential([]) -> [];
bitonic_connect_sequential([X]) -> [X];
bitonic_connect_sequential(Xs) ->
    {Left, Right} = lists:split(length(Xs) div 2, Xs),
    Mins = [min(A,B) || {A,B} <- lists:zip(Left, Right)],
    Maxs = [max(A,B) || {A,B} <- lists:zip(Left, Right)],
    Minsconnected = bitonic_connect_sequential(Mins),
    Maxsconnected = bitonic_connect_sequential(Maxs),
    lists:append (Minsconnected, Maxsconnected).

bitonic_sort(Xs, NumProcs) ->
  L = length(Xs) div NumProcs,
  if
    L =< 2 ->
        bitonic_sort_sequential(Xs);
    true ->
        {Left, Right} = lists:split(L, Xs),
        Parent = self(),
        R = make_ref(),
        spawn_link(fun() -> Result = lists:reverse (bitonic_sort(Right, NumProcs)), Parent ! {R, Result} end),
        LeftSorted = bitonic_sort(Left, NumProcs),
        RightSorted = receive
        {R, Result} -> Result
    end,
    bitonic_connect (lists:append(LeftSorted, RightSorted), NumProcs)
end.

bitonic_sort_sequential([]) -> [];
bitonic_sort_sequential([X]) -> [X];
bitonic_sort_sequential(Xs) ->
    L = length(Xs),
    {Left, Right} = lists:split(L div 2, Xs),
    LeftSorted = bitonic_sort_sequential(Left),
    RightSorted = lists:reverse(bitonic_sort_sequential(Right)),
    bitonic_connect_sequential (lists:append(LeftSorted, RightSorted)).

% генерация массива длины 2^N
generate_array(N) -> 
    gen_array(trunc(math:pow(2, N))).

gen_array(0) -> 
    [];
gen_array(N) -> 
    [rand:uniform(100) | gen_array(N - 1)].
