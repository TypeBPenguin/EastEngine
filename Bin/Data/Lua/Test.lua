function Sum(a, b)
  --return a + b, a + a + b + b;
  
  local result = 0;
  for i = 1, 10000 do
    result = 0;
    for i = 1, 10000 do
      result = result + i;
    end
  end
  
  return a, b;
end

function Minus(a, b)
  return a - b;
end

function Multiply(a, b)
  return a * b;
end

function Devide(a, b)
  return a / b;
end