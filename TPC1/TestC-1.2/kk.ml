let rec combinations l n = 
 if n = 0 then [[]]
 else match l with
       | []    -> []
       | x::xs -> [x]::combinations xs n
;;

let rec combinations2 l n = 
 
 if n <= 1 then combinations l n
 else match l with
       | []    -> []
       | x::y::xs -> (x::y::[])::combinations2 (y::xs) n1
;;