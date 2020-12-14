#####pull in and parse data
from os import walk
import os
import time

from collections import deque   

import math


STATE_SIZE = 12 #number of item/tuples to print
STATE_ELEMENTS = 24 #number of items, tuples expanded
ACTION_SIZE = 7 #number of items
HISTORY_SIZE=30

startTime = time.time()


directoryString = "PATH OF RAW DATA"
cleanedDirectoryString = "PATH OF CLEANED DATA"

files = []
for (dirpath, dirnames, filenames) in walk(directoryString):
    files.extend(filenames)
    break

MAX_LINES = 300000 #100,000 is about 38Mb
    
   
    
#these magic numbers are pulled from https://github.com/RLBot/RLBot/wiki/Useful-Game-Values
#we want everything [-1, 1]
def normalizeStateLine(line):
    
    #location normalization just uses absolute field dimensions. Technically, valid states probably
    #are a tiny bit smaller as hit boxes wouldn't let you get right up to the edge. I think 
    #will still work?
    
    line[1] /= 4096.0 #car loc x
    line[2] /= 5120.0 #car loc y
    line[3] /= 2044.0 #car loc z
    
    line[4] /= (math.pi/2.0) #car pitch
    line[5] /= math.pi     #car yaw
    line[6] /= math.pi     #car roll
    
    #assumes boost speed it maximum possible in all dimensions
    line[7] /= 2300.0 #car vel x
    line[8] /= 2300.0 #car vel y
    line[9] /= 2300.0 #car vel z
    
    line[10] /= 5.5 #car angVel x
    line[11] /= 5.5 #car angVel y
    line[12] /= 5.5 #car angVel z
    
    line[17] /= 100.0
    
    line[18] /= 4096.0 #ball loc x
    line[19] /= 5120.0 #ball loc y
    line[20] /= 2044.0 #ball loc z

    line[21] /= 6000.0 #ball vel x
    line[22] /= 6000.0 #ball vel y
    line[23] /= 6000.0 #ball vel z
    
    return line


def listToCsv(list):
    m = map(str, list)
    return ','.join(m)
    
    

for file in files:
    name = file.split(".")[0]
    print("parsing file "+str(name))
    file = directoryString + "\\" + file
    fileSize = os.stat(file).st_size
    
    states = []
    actions = []
    
    with open(file) as f:
    
        suffixCount = 0
        saveFile = open(cleanedDirectoryString+name+"-"+str(suffixCount)+".log",  "w")        
    
        recentSA = deque (maxlen=HISTORY_SIZE)
    
        dataCount = 0
        lineIndex = 0
        skippedCount = 0
        
        stateItem = -1
        actionItem = -1
        
        for line in f:
        
            itemCount = len(line.split(";"))
            
            #put data into correct format (list of floats)
            line = line.replace(";", ",")
            line = line.replace("True", "1")
            line = line.replace("False", "0")
            line = line.strip()
            line = line.split(",")
            line = [float(i) for i in line]
            
            #self/ball state
            if(lineIndex == 0):
                assert itemCount == STATE_SIZE, "WARNING: "+str(f)+", strange line setup at "+str(lineIndex+1)

            
             #   line = normalizeStateLine(line) 
                stateItem = line     
               
            #enemy line    
            elif (lineIndex == 1):
            
            #remove comments and pass to include opponent data in state data
                pass 
            
            #    assert stateItem != -1                
            #    stateItem = stateItem + line[1:4]
            #    assert len(stateItem) == 27
                
                
                
                
                
            #action line
            elif (lineIndex == 2):
                assert itemCount == ACTION_SIZE, "WARNING: "+str(f)+", strange line setup at "+str(lineIndex+1)
                actionItem = line
                
                assert stateItem != -1
                
                saPair = (stateItem, actionItem)
                if(recentSA.count(saPair) == 0):
                    recentSA.append(saPair)
                    states.append(saPair[0])
                    actions.append(saPair[1])
                else:
                #    print("element already exists, skipping...")
                    skippedCount += 1
                
                stateItem = -1                
                actionItem = -1
                
                
                dataCount += 1
                   
                
            if((dataCount+1) % MAX_LINES == 0):
                print("writing new sub file "+str(suffixCount+1))
                assert len(states) == len(actions), str(len(states)) +" vs "+ str(len(actions))
                for i in range(len(states)):
                
                    saveFile.write(listToCsv(states[i]))
                    saveFile.write("\n")
                    saveFile.write(listToCsv(actions[i]))
                    saveFile.write("\n")
                    
                    
                states = []
                actions = []
            
                saveFile.close ()
                suffixCount += 1
                saveFile = open(cleanedDirectoryString+name+"-"+str(suffixCount)+".log",  "w")  
                print(str(dataCount) + " file items, skipped "+ str(skippedCount))
                dataCount = 0
                skippedCount = 0
            
                  
            lineIndex += 1  
            if(lineIndex >= 3):
                lineIndex = 0
        

        #verify correct data amounts
        print("end of file")
        assert len(states) == len(actions),"different state/actions lengths "+str(len(states[0]))+"/"+str(len(actions[0]))           
        

        for i in range(len(states)):
            saveFile.write(listToCsv(states[i]))
            saveFile.write("\n")
            saveFile.write(listToCsv(actions[i]))
            saveFile.write("\n")

        saveFile.close ()
        
print("total minutes: " +str((time.time() - startTime)/60))


