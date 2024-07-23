import os
import time
import random

cntRand = 8
digitRand = 1000
cntLoop = 10


def provideData():
    s = ""
    for _ in range(cntRand):
        num = random.randint(-digitRand, digitRand)
        s = s + str(num) + ','  
        #data.append(random.randint(-digitRand, digitRand)
    s += '\0'
    return s

def sortName():
    dic = {1:"insert", 2:"shell", 3:"select", 4:"bubble", 5:"quick", 6:"merge"}
    key = random.randint(1, 6)
    return dic[key]

def ascORdes():
    dic = {0:"asc", 1:"des"}
    key = random.randint(0,1)
    return str(dic[key])

for _ in range(cntLoop):
    command = "./client 127.0.0.1 {} {} {}".format(provideData()[:-2], sortName(), ascORdes())
    #print(command)
    begin = time.time()
    os.system(command)
    t = time.time() - begin
    print("sort has spent {}s\n".format(t))
    time.sleep(1.02)
