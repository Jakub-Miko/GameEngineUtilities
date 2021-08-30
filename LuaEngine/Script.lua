a = 8 + 2

function add(num1, num2)
Context.SendMessage(Context.GetInput())	
print(Context.GetX());
return Context.CSum(num1,num2)
end

function concatenate(str1, str2)
	return str1 .. str2
end


function Test(second) 
X = GetX()
Y = GetY()

Z = GetInput()


SetX(CSum(X,Y))
SetY(CSum(X,Y) + tonumber(Z))

copy_values(second)

SendMessage("Hello World")

end




function flt_sm(n1,n2) 


return FloatSum(n1,n2)
end