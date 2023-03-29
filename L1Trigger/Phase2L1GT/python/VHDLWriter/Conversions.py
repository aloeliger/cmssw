import L1Trigger.Phase2L1GT.VHDLWriter.Conditions as cond
import L1Trigger.Phase2L1GT.VHDLWriter.Writer as writer




def checkFilter(filt):
    """
    takes in a cmssw process object and
    checks if filter is a known gt condition
    converts it to corresponding Condition object defined in 
    Conditions.py
    """
    knownConditions = [cond.SingleObjCond, cond.DoubleObjCond, cond.TripleObjCond,cond.QuadObjCond]
    Condit = None
    for Condition in knownConditions:
        try:
            if filt.type_() == Condition.Label:
                Condit = Condition()
        except Exception as ex:
            print(ex)
            pass

    if Condit == None:
        return 0

    collections = Condit.getCollections(filt)
    for idx, col in collections.items():
        knowncuts = Condit._cut_aliases.keys()
        for knowncut in knowncuts:
            if col.hasParameter(knowncut):
                Condit.setCut(knowncut,col.getParameter(knowncut).value(), idx,Condit.NumberOfCollections)
                Condit.addResources(knowncut)
    if ((Condit.Label == "L1GTTripleObjectCond" ) or (Condit.Label == "L1GTQuadObjectCond"))  :
        for idx, col in Condit.getCorrelations(filt).items():
            knowncuts = Condit._cut_aliases.keys()
            for knowncut in knowncuts:
                if col != None:
                    if col.hasParameter(knowncut):
                        Condit.setCut(knowncut,col.getParameter(knowncut).value(), idx,Condit.NumberOfCorrelations)
                        Condit.addResources(knowncut)
    knowncuts = Condit._cut_aliases.keys()
    for knowncut in knowncuts:
        if filt.hasParameter(knowncut):
            Condit.setCut(knowncut, filt.getParameter(knowncut).value())
            Condit.addResources(knowncut)
    for idx, tag in enumerate(Condit._InputTags):
        Condit._setInputObject(idx + 1, tag.productInstanceLabel)

    return Condit
def assignAlgoBits(obj):
    """
    takes in 
    """
    Algobitlist = obj.channelConfig.value().pop().getParameter('algoBits')
    algos = cond.defineAlgoBits()
    for Bititem in Algobitlist:
        name = Bititem.bitPos.value()
        value = Bititem.path.value()
        algos.SetBit(value,name)
    return algos


def getConditionsfromConfig(obj):
    filterdict = {}
    for key,value in  obj.filters.items():
        x = checkFilter(value)
        if x != 0:
            filterdict[key] = x 
            addPathsToModule(obj,key,filterdict[key])
    return filterdict

def findPaths(obj,modulename):
    pathlist = []
    for path in obj.paths.values():
        if modulename in path.moduleNames():
            pathlist.append(path.label())
    return pathlist


def getModulesfromPath(gt,algobits):
    modules = {}
    for key,value in algobits.Assignment.items():
        modules[key] = gt.paths[key].moduleNames()
    return modules


def checkiflogicalcombination(filterlist,algobits):
    filterpaths =  [path for filt in filterlist for path in filt.Path]
    combinatorialpath = []
    for algopath in algobits.keys():     
        if algopath not in filterpaths:
            combinatorialpath.append(algopath)
    return combinatorialpath



def getLogicalFilters(gt,knownfilters):
    combinatorialfilters = {} 
    for path in gt.paths:
        pathmodules = gt.paths[path].moduleNames()
        for module in pathmodules:
            if (module in gt.filterNames().split())  :
                if gt.filters[module].type_() == "PathStatusFilter":
                    expression = gt.filters[module].getParameter("logicalExpression").value()
                    containsmodules = getModules(gt,expression)
                    for mod in containsmodules:
                         if mod in knownfilters.keys():
                                combinatorialfilters[path] = cond.LogicalFilter(module,expression,containsmodules)
                                break

    return combinatorialfilters
def getModules(obj,expression):
    words = expression.split()
    modulelist = []
    moduleset = {}
    for word in words:
        try:
            moduleset = (obj.paths[word].moduleNames())
        except:
            pass
        if moduleset != set():
            modulelist.append(moduleset.pop())
        if moduleset != set():
            print("warning %s is part of a combinatorial logic and its path contains more than 1 modules, this will lead to undefined behaviour".format(word))
    return modulelist



def associatePathAndModules(gt,knownfilters):
    pathswithmodules = {}    
    for path in gt.paths:
           x = gt.paths[path].moduleNames()
           for module in x:
               for filt in knownfilters.keys():
                   if filt == module:
                        pathswithmodules.setdefault(filt,[]).append(path)
    return pathswithmodules


def addPathsToModule(gt,filtername,filtervalue):    
    for path in gt.paths:
           modules = gt.paths[path].moduleNames()
           for module in modules:
                if filtername == module:
                    filtervalue.addPath(path)



def writeAlgoblocks(conditions,logfilt):
    algodict = cond.Algorithmsdict()
    algodict.addLogicalFilters(logfilt)
    algodict.addConditions(conditions)
    return algodict

def distributeAlgos(algodict,numslrs):
    algounits = []
    for i in range(numslrs):
        algounits.append(cond.AlgorithmBlock())
    cnt = 0

    addmax = algodict.popMaxalgoblock()
    if(numslrs == 1):
        while(addmax != 0):
            algounits[cnt].Combineblocks(addmax)
            addmax = algodict.popMaxalgoblock()
        return algounits
    else:
        print(type(addmax))
        if(addmax == type(int)):
            return addmax
        slrcounter = 0
        repeater = 0
        while(addmax != 0):
            print(cnt)
            if cnt < (numslrs - 1):
                algounits[cnt].Combineblocks(addmax)
            elif cnt == (numslrs - 1):
                algounits[cnt].Combineblocks(addmax)
            else:
                algounits[6 - cnt].Combineblocks(addmax)
            
            if (((cnt == numslrs - 1)) and (repeater > 0)):
                cnt = cnt
                repeater = 0
            elif (cnt == 2 * (numslrs - 1)):
                cnt = 0
                repeater = 1
            else:
                cnt = cnt + 1
                repeater = 1
            addmax = algodict.popMaxalgoblock()
        return algounits


def distributeAlgosWithoutopt(algodict,numslrs):
    algounits = []
    for i in range(numslrs):
        algounits.append(cond.AlgorithmBlock())

    if(numslrs == 1):
        while(addalgo != 0):
            algounits[0].Combineblocks(addalgo)
            addalgo = algodict.popMaxalgoblock()
        return algounits
    else:
        addalgo = algodict.algoblocks.pop()
        print(type(addalgo))
        if(addalgo == type(int)):
            return addalgo
        cnt = 0
        repeater = 0
        while(addalgo != None):
            if addalgo == type(int):
                return 0
            print(cnt)
            if cnt < (numslrs - 1):
                algounits[cnt].Combineblocks(addalgo)
            elif cnt == (numslrs - 1):
                algounits[cnt].Combineblocks(addalgo)
            else:
                algounits[6 - cnt].Combineblocks(addalgo)
            
            if (((cnt == numslrs - 1)) and (repeater > 0)):
                cnt = cnt
                repeater = 0
            elif (cnt == 2 * (numslrs - 1)):
                cnt = 0
                repeater = 1
            else:
                cnt = cnt + 1
                repeater = 1

            if algodict.algoblocks == []:
                return algounits
            else:
                addalgo = algodict.algoblocks.pop()
        return algounits

def distributeAlgosAtRandom(algodict,numslrs,seed = 2):
    import random as rand
    rand.seed = seed

    algounits = []
    for i in range(numslrs):
        algounits.append(cond.AlgorithmBlock())
    addalgo = algodict.algoblocks.pop()
    while(algodict.algoblocks != []):
        algounits[rand.randint(0,(numslrs - 1))].Combineblocks(addalgo)
        addalgo = algodict.algoblocks.pop()
    return algounits

def assignAlgostoSlrs(knownfilters,logicalcombinations,numslrs):
    algoblocks = WriteAlgoDict(knownfilters,logicalcombinations)
    distributedAlgos = distributealgos(algoblocks,numslrs)
    return distributedAlgos



def writeAlgounits(distributedAlgos,algomap,knownfilters,logcomb):
    for index,value in enumerate(distributedAlgos):
        modules = dict()
        paths = dict()
        condtext = ""
        algounittext = ""
        bits = {}
        tdistributedAlgos = {}
        logicalcombinations = dict()
        for mod in value.Modules:
            condtext += writer.conditionwriter(mod,knownfilters[mod])
            tdistributedAlgos[mod] = knownfilters[mod]
        if(value.LogicalPath != set()):
            for log in value.LogicalPath:
                logicalcombinations[log] = logcomb[log]
        algounittext = writer.algounitWriter(algomap,condtext,tdistributedAlgos,logicalcombinations,index)
        writer.writeAlgounitToFile("p2gt_algos_slr{}.vhd".format(index),algounittext)


def getAlgobits(algomap,distributedalgos,Outputchans):
    chans = Outputchans
    algosdict = {}
    for block in distributedalgos: 
        algochan = chans.pop(0)
        algodict = {}
        for key,conf in algomap.items():
            if(conf in block.Paths or conf in block.LogicalPath):
                algodict[key] = conf
        algosdict[algochan] = algodict
    return algosdict

