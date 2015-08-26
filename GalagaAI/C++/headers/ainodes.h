#ifndef AINODES_H
#define AINODES_H

#include <string>
#include <vector>

class MyTreeNode{
		public:
	MyTreeNode * parent;
	std::string nodeName;
	std::string status;
	std::string nodeType;
	std::vector<MyTreeNode *> children;

	virtual std::string execute();
};

//basic selector that forms branches of tree
//is used as a starting point for all branch node types
class SelectorNode : public MyTreeNode{
public:
	SelectorNode();
	SelectorNode(SelectorNode * p);
	std::string traverseChildren();
};


//does actions
class dodgeNode : public MyTreeNode{
public:	
	dodgeNode(SelectorNode * p); 
	std::string execute();
};

class fireNode : public MyTreeNode{
public:
	fireNode(SelectorNode * p); 
	std::string execute();
};


class adjustNode : public MyTreeNode{
public:
	adjustNode(SelectorNode * p);
	std::string execute();
};

class moveNode : public MyTreeNode{
public:
	moveNode(SelectorNode * p);
	std::string execute();
};


class prioritySelectorNode : public SelectorNode{
		public:
	prioritySelectorNode(SelectorNode * node);
	std::string execute();
};

class concurrentSelectorNode: public SelectorNode{
		public:
	concurrentSelectorNode(SelectorNode * node); 
	std::string execute();
};

class runningSelectorNode : public SelectorNode{
		public:
	runningSelectorNode(SelectorNode * node);
//	std::string execute();
	std::string runningCheck();
}; 


class behaviorTree{
		public:
	MyTreeNode * root;

	behaviorTree();
	std::string resetTree(MyTreeNode * node);
	void runTree();
};
#endif
