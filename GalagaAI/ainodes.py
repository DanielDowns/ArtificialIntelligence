import time

#does the stuff :P
class actionNode (object):
    def __init__(self, par, exeFunc):
        self.status = "ready"
        self.parent = par
        self.execute = exeFunc
        self.parent.children.append(self)
             
 
#basic selection that forms branches of tree
class SelectorNode (object):     
    def __init__(self, par):
        self.status = "ready"
        self.children = []
        if par is not None:
            self.parent = par
            self.parent.children.append(self)
             
    #allows priority execution of children and stops when failed or running   
    def traverseChildren(self):
        result = ""
        for child in self.children:
            result = child.execute()
            if(result == "failed" or result == "running"):
                return result
             
        return "completed"        
                          
             
             
#traverses the children in priority, regardless of already running nodes
class prioritySelectorNode (SelectorNode):     
             
    def execute(self):
        super(prioritySelectorNode, self).traverseChildren()
      
                    
#traverses all children regardless of return conditions                   
class concurrentSelectorNode (SelectorNode):
    def __init__(self, par):
        super(concurrentSelectorNode, self).__init__(par)
 
    def execute(self):
        for child in self.children:
            result = child.execute()
                    
 
#checks for running nodes before a priority traversal               
class runningSelectorNode (SelectorNode):
    def __init__(self, par):
        super(runningSelectorNode, self).__init__(par)
             
    #returns the first child that's "running" 
    def runningCheck(self):
        for child in self.children:
            if(child.status == "running"):
                return "running"
                          
        return -1
 
    def execute(self):
        runner = self.runningCheck()
        if(runner != -1):
            runResult = runner.execute()
            return runResult
        else:
            SelectorNode.traverseChildren()
             
 
 
      
class behaviorTree:    
    
    def __init__(self):
        self.root = None 
 
    #resets all states to "ready", ignoring all that are "running"
    def resetTree(self, node):
        retString = ""
      
        if(not isinstance(node, actionNode)):     
            nodeStatus = ""
                    
            for child in node.children:
                kidStatus = self.resetTree(child)
                if(kidStatus == "running"):
                    nodeStatus = "running"
                    
            if(nodeStatus != "running" or node == self.root):
                node.status = "ready"
                retString = "ready"
            else:
                retString = "running"
                          
        elif(isinstance(node, actionNode)):
            if node.status != "running":
                node.status = "ready"
                retString = "ready"
            else:
                retString = "running"
             
        return retString
    
    def runTree(self):
        while(True):
            t0 = time.time()
            self.root.execute()
            self.resetTree(self.root)
            print "" #blank line in between runs
            t1 = time.time()
            
            print "1 cycle time: " + str(t1 - t0)