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




SetX(CSum(X,Y))
SetY(CSum(X,Y))

copy_values(second)

SendMessage("Hello World")

end




function flt_sm(n1,n2) 

return FloatSum(n1,n2)
end



table1 = {}


function table1:OnUpdate(num1, num2)
return num1 + num2
end

function ObjectTest(value) 
	print(value.x)
	print(value.y)
	return {x = 9, y = 9}
end


table2 = {}

function table2:OnUpdate()
SendMessage("OnUpdate2")
end
