import math
 
def point_enemy_reward(objects, point):
    
    OBJECT_SIZE = 40 #rough pixel size of all enemies, should be swapped to actual
    OBJECT_REWARD = 20 #random value of enemies, should be swapped to actual
    
    for key in objects:
        if key != "missile":
            for item in objects.get(key):            
                if point > item[0] - (OBJECT_SIZE/2) and point < item[0] + (OBJECT_SIZE/2):
                    return OBJECT_REWARD
         
    return 0
                          
                          
                          
#doesn't yet take into account hitting the side of the ship. FIX IT               
def point_threat(threats, point):
    
    LARGE_CONSTANT = 100 #a high value that should overpower the formula if whole
    
    highest = 0
    for threat in threats:
        
        #doesn't account for missiles being small
        if((threat[1] <= point and point <= threat[3]) or (threat[1] >= point and point >= threat[3])):
            if(threat[4] == 0):
                return 100
            danger = LARGE_CONSTANT/(threat[4])  
            if danger > highest:
                highest = danger
                              
    return highest
      
      
      
#assumes a super position and ignores time to travel
#will work but be very sub-optimal at harder levels   
def point_travel_threat(threatList):
    sumThreat = 0
    for threat in threatList:
        sumThreat += threat
             
    return sumThreat
      
      
 
def point_distance(point, playX):
    return math.floor(abs(point - playX)/50) #to make it not as important
      
 
 
def point_tactical_value(point):
    
    TACTICAL_MIN = 100  #whole screensize points that flank the middle 1/3
    TACTICAL_MAX = 350  #of player move line
    
    if point > TACTICAL_MIN and point < TACTICAL_MAX:
        return 10
    else:
        return 0