require "Json"

package.path = package.path .. ";./?.lua"

function test1()
    for i = 1, 10 do
        print("In Test1: " .. tostring(i))
    end
end


function test2(first, last)
    for i = first, last do
        print("In Test2: " .. tostring(i))
    end
end

function sum2(a, b)
    local result = a + b
    return result
end


function sum3(a, b, c)
    return a + b + c
end

function upvaltest(a, b, c)
    function plusa(d)
        return a + d
    end

    function plusb(d)
        return b + d
    end

    function another(d, f)
        function andagain(g)
            print("a, d, g: " .. a .. ", " .. d .. ", " .. g)
        end

        print("andagain(" .. d .. ", " .. f .. "): " .. andagain(d, f))
    end

    print ("Plusa(" .. a .. ", 10): " .. plusa(10))
    print ("Plusb(" .. b .. ", 15): " .. plusb(15))
    print ("another(" .. b .. ", " .. c .. "): " .. another(b, c))
end

function abctotable(a, b, c)
    local tab = {["a"] = 1, ["b"] = 2, ["c"] = {1, 2}, ["d"] = {["a"] = 1, ["b"] = 2}}
    local output = {["a"] = "a", ["b"] = "b", ["c"] = "c"}
    local d = 1
    local e = 3

    output[1] = 1
    output[2] = 2
    output[3] = 4

    print ("An extra line of output...")

    print("Param 1: " .. a)

    print("Param 2: " .. b)

    print("Param 3: " .. c)

    print("Sum2(a, b) = " .. sum2(a, b))

    print("Sum3(a, b, c) = " .. sum3(a, b, c))

    return output
end

