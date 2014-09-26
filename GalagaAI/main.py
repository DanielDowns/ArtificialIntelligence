import ainodes
import dodgealg
import imagescreen
import keyboard
import time
from multiprocessing import freeze_support

pastObjects = {}
playerPos = 0 
targetPoint = 300
movement = "none"
 
#dodge threat 
def dodgeExecute():

    global playerPos
    global pastObjects
    global targetPoint
    
    #timing code
    
    
    objects = pastObjects
    t0 = time.time()
    newObjects, threats, playerPos = imagescreen.parseScreen(objects)
    t1 = time.time() 
    
    
    playerPos += 20 #location returned is top left and needs to be center
    pastObjects = newObjects
   
    if(threats is not None):
        if(len(threats) == 0):
            return "complete"
        else:
            print ""
    else:
        return "complete"
    
    print "dodging!"
    maxP = -1000 #keeps track of value of best location to move to
      
    #look right then left bc the "score" of spots is dependent on inner spot threat
    Tpoints = []
    debugPoints = []
    
    playerLine = list(range(39,476))
    
    rightLine = playerLine[playerPos:]
    leftLine = playerLine[:playerPos]
    leftLine.reverse() #need to traverse the list in desecending order (away from ship)
        
    for point in rightLine:
        AthreatVal = dodgealg.point_threat(threats,point)
        ArewardVal = dodgealg.point_enemy_reward(pastObjects, point)
        AtravelThreat = dodgealg.point_travel_threat(Tpoints) 
        AdistanceVal = dodgealg.point_distance(point, playerPos)
        AtacticalVal = dodgealg.point_tactical_value(point) 
        
        P = ArewardVal - AthreatVal - AtravelThreat - AdistanceVal + AtacticalVal               
        Tpoints.append(AthreatVal)
        debugPoints.append(P)
        if(P > maxP):
            maxP = P
            targetPoint = point
                
#    print "right values"            
#    print debugPoints
    debugPoints = []
 
    Tpoints = []
    for point in leftLine:
        AthreatVal = dodgealg.point_threat(threats,point)
        ArewardVal = dodgealg.point_enemy_reward(pastObjects, point)
        AtravelThreat = dodgealg.point_travel_threat(Tpoints) #is currently excluded. bug where every point would add threat value, causing a too-high rise in travel threat
        AdistanceVal = dodgealg.point_distance(point, playerPos)
        AtacticalVal = dodgealg.point_tactical_value(point) 
        
        P = ArewardVal - AthreatVal - AdistanceVal + AtacticalVal    
        Tpoints.append(AthreatVal)
        debugPoints.append(P)
        if(P > maxP):
            maxP = P
            targetPoint = point
    
    if(playerPos == targetPoint):
        return "complete"
                 
    return "running"
 
#move to target 
def adjustExecute():
    
    global playerPos
    global targetPoint
    global pastObjects
    
    
    print "adjusting!"
    objects = pastObjects
    playerPos += 20 #location returned is top left and needs to be center
    
    min = 1000000
    lock = None
    
    
    for type in objects:
        if type == "missile":
            continue
        for target in objects.get(type):
            dis = abs(playerPos - target[0])
            if dis < min:
                min = dis
                lock = target
 
    if(lock is not None):
        targetPoint = lock[0]     
  
  
#    print "adust targetpoint: " + str(targetPoint) +" vs player: " + str(playerPos)
            
#control use            
def movementExecute():
    global playerPos
    global targetPoint
    global movement
    
    dif = targetPoint - playerPos
    dif = abs(dif)
    
    print "current point: " + str(playerPos) + "//target point: " + str(targetPoint)
    
    if(targetPoint < playerPos and dif > 2):
        movement = keyboard.MoveLeft(movement)
        return "running"
    elif(targetPoint > playerPos and dif > 2):    
        movement = keyboard.MoveRight(movement) 
        return "running"
    else:
        print "stopping"
        movement = keyboard.StopMoving(movement, "none")
        return "complete"
 
def fireExecute():
    global pastObjects
    objects = pastObjects
    
    for type in objects:
        if type == "missile":
            continue
        for target in objects.get(type):
            if abs(playerPos - target[0]) <= 10:
                keyboard.Fire() 
                return "complete"
    return "complete"

if __name__ == "__main__":
    freeze_support()
    tree = ainodes.behaviorTree()
     
    AI = ainodes.concurrentSelectorNode(None)
     
    move = ainodes.prioritySelectorNode(AI)
    dodge = ainodes.actionNode(move, dodgeExecute)
    adjust = ainodes.actionNode(move, adjustExecute)
    
    executeMovement = ainodes.actionNode(AI, movementExecute) 
    fire = ainodes.actionNode(AI, fireExecute)
    
    tree.root = AI
    tree.runTree()
