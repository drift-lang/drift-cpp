import dis

x = 45

if x > 0:
    for i in range(0, 10):
        n = x + 2

print(x)
print(n)

x = 56

print(x)
print(n)

def foo():
    x = 46
    x = 67
    x = 14
    return x

x = foo()
print(x)

map