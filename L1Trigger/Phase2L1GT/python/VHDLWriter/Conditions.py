from ast import Mod
from modulefinder import Module
from queue import Empty
from turtle import update
from L1Trigger.Phase2L1GT.l1tGTScales import l1tGTScales

class Condition:
    """
    Base Class for all EDFilters that correspond to an P2GT condition
    Contains: Cuts, The VHDL Resource usage, Paths corresponding to the Modules in the config
    """
    _ObjectNameConversions = {
        "GTTPromptJets" : "GTT_PROMPT_JETS_SLOT",
        "GTTDisplacedJets" : "GTT_DISPLACED_JETS_SLOT",
        "GTTPromptHtSum" : "GTT_HT_MISS_PROMPT_SLOT",
        "GTTDisplacedHtSum" : "GTT_HT_MISS_DISPLACED_SLOT",
        "GTTEtSum" : "GTT_ET_MISS_SLOT",
        "GTTTaus" : "GTT_TAUS_SLOT",
        "GTTPhiCandidates" : "GTT_PHI_CANDIDATE_SLOT",
        "GTTRhoCandidates" : "GTT_RHO_CANDIDATE_SLOT",
        "GTTBsCandidates" : "GTT_BS_CANDIDATE_SLOT",
        "GTTPrimaryVert" : "GTT_PRIM_VERT_SLOT",
        "CL2Jets" : "CL2_JET_SLOT",
        "CL2HtSum" : "CL2_HT_MISS_SLOT",
        "CL2EtSum" : "CL2_ET_MISS_SLOT",
        "CL2Taus" : "CL2_TAU_SLOT",
        "CL2Electrons" : "CL2_ELECTRON_SLOT",
        "CL2Photons" : "CL2_PHOTON_SLOT",
        "GCTNonIsoEg" : "GCT_NON_ISO_EG_SLOT",
        "GCTIsoEg" : "GCT_ISO_EG_SLOT",
        "GCTJets" : "GCT_JETS_SLOT",
        "GCTTaus" : "GCT_TAUS_SLOT",
        "GCTHtSum" : "GCT_HT_MISS_SLOT",
        "GCTEtSum" : "GCT_ET_MISS_SLOT",
        "GMTSaPromptMuons" : "GMT_SA_PROMPT_SLOT", 
        "GMTSaDisplacedMuons" : "GMT_SA_DISPLACED_SLOT", 
        "GMTTkMuons" : "GMT_TK_MUON_SLOT", 
        "GMTTopo" : "GMT_TOPO_SLOT"
        }
        


    def __init__(self):
        self.Cuts = {}
        self.InputObjects = {}
        self._InputTags = []
        self.Paths = []
        self.ResourceUseage = CutResources()
        self.ResourcesperCut = {
            ('minPt') : CutResources(bram = 0 , dsp = 0, lut = 110),
            ('minEta','maxEta') : CutResources(bram = 0 , dsp = 0, lut = 120),
            ('minPhi','maxPhi') : CutResources(bram = 0 , dsp = 0, lut = 123),
            ('minZ0','maxZ0') : CutResources(bram = 0 , dsp = 0, lut = 123),
            ('qual') : CutResources(bram = 0 , dsp = 0, lut = 40),
            ('iso') : CutResources(bram = 0 , dsp = 0, lut = 40)

        }
        self._HWConversionFunctions = {
            'minPt': l1tGTScales.to_hw_pT,
             'minEta': l1tGTScales.to_hw_eta,
             'maxEta': l1tGTScales.to_hw_eta,
             'minPhi': l1tGTScales.to_hw_phi,
             'maxPhi': l1tGTScales.to_hw_phi,
            'minZ0': l1tGTScales.to_hw_z0,
            'maxZ0': l1tGTScales.to_hw_z0,
        }

        self._cut_aliases = {

        }
    def booltostring(self,x):
        if x == 1:
            return True
        else : 
            return False

    def addResources(self,knowncut):
        for k in list(self.ResourcesperCut.keys()):
            if knowncut in k:
                if k in self.ResourcesperCut.keys():
                    self.ResourceUseage.addCutResources(self.ResourcesperCut.pop(k))




    def _setInputObject(self,condition,value):
        self.InputObjects[condition] = self._ObjectNameConversions.get(value)

    def setCut(self,key,physvalue, collection = "",numberofparameters = 0):

        if collection != "":
            if self.getHWCut(key) not in self.Cuts.keys():
                cut = _Cut()
                cut.setNumberofvalues(numberofparameters)
                if key in self._HWConversionFunctions:
                    if ((key == 'ss') or (key == 'os')):
                        cut.setforBooleanCut(numberofparameters)
                    cut.setCutat(self._HWConversionFunctions[key](physvalue),physvalue,collection)
                    self.Cuts[self.getHWCut(key)] = cut
                else:
                    cut.setCutat(physvalue,physvalue,collection)
                    self.Cuts[self.getHWCut(key)] = cut
            else:
                if key in self._HWConversionFunctions:
                    self.Cuts[self.getHWCut(key)].setCutat(self._HWConversionFunctions[key](physvalue),physvalue,collection)
                else:
                    self.Cuts[self.getHWCut(key)].setCutat(physvalue,physvalue,collection)       
        else:
            cut = _Cut()
            if key in self._HWConversionFunctions:
                cut.setCut(self._HWConversionFunctions[key](physvalue),physvalue)
                self.Cuts[self.getHWCut(key)] = cut
            else:
                cut.setCut(physvalue,physvalue)
                self.Cuts[self.getHWCut(key)] = cut

    def setName(self,name):
        self.Name = name
    def addPath(self,path):
        self.Paths.append(path)

    def getHWCut(self, cut, collection = ""):
        if cut in self._cut_aliases:
            return self._cut_aliases[cut].format(collection)
        return cut

    def getCollections(self, object):
        return {}

    def setInputObjects(self, **inobjs):
        for name, value in inobjs.items():
            if name in self._InputTags:
                self.InputObjects[name] = value
    def _setHWConversionFunctions(self,indict):
           self._HWConversionFunctions = indict


class DoubleObjCond(Condition):
    """
    Class for to the L1GTDoubleObjectCond 
    """
    Label = "L1GTDoubleObjectCond"
    Template = "double.template"
    NumberOfCollections = 2 
    NumberOfCorrelations = 2
    def __init__(self):
        Condition.__init__(self)
        
        self._HWConversionFunctions.update({
            'minDEta': l1tGTScales.to_hw_eta,
            'maxDEta': l1tGTScales.to_hw_eta,
            'minDPhi': l1tGTScales.to_hw_phi,
            'maxDPhi': l1tGTScales.to_hw_phi,
            'minDR': l1tGTScales.to_hw_dRSquared,
            'maxDR': l1tGTScales.to_hw_dRSquared,
            'minInvMass': l1tGTScales.to_hw_InvMassSqrDiv2,
            'maxInvMass': l1tGTScales.to_hw_InvMassSqrDiv2,
            'minTransMass': l1tGTScales.to_hw_TransMassSqrDiv2,
            'maxTransMass': l1tGTScales.to_hw_TransMassSqrDiv2,
            'minCombPt' :   l1tGTScales.to_hw_PtSquared,
            'maxCombPt' :   l1tGTScales.to_hw_PtSquared
        })

        self._cut_aliases.update({ 
            'minPt' : 'pT_cuts',
            'minEta' : 'minEta_cuts',
            'maxEta' : 'maxEta_cuts',
            'minPhi' : 'minPhi_cuts',
            'maxPhi' : 'maxPhi_cuts',
            'minZ0' : 'minZ0_cuts',
            'maxZ0' : 'maxZ0_cuts',
            'qual' : 'qual_cuts',
            'iso' : 'iso_cuts',
            'minDEta' : 'mindEta_cut',
            'maxDEta' : 'maxdEta_cut',
            'minDPhi' : 'mindPhi_cut',
            'maxDPhi' : 'maxdPhi_cut',
            'minDR' : 'mindRSquared_cut',
            'maxDR' : 'maxdRSquared_cut',
            'minInvMass' : 'mininvMassSqrDiv2_cut',
            'maxInvMass' : 'maxinvMassSqrDiv2_cut',
            'minTransMass' : 'mintransMassSqrDiv2_cut',
            'maxTransMass' : 'maxtransMassSqrDiv2_cut',
            'minCombPt' : 'minPTSqr_cut',
            'maxCombPt' : 'maxPTSqr_cut',
            'os' : 'os_cut',
            'ss' : 'ss_cut'
        })
        self.ResourcesperCut.update({
            ('minDEta','maxDEta') : CutResources(bram = 0 , dsp = 0, lut = 800),
            ('minDPhi','maxDPhi') : CutResources(bram = 0 , dsp = 0, lut = 800),
            ('minDR','maxDR') : CutResources(bram = 0 , dsp = 24, lut = 1800),
            ('minInvMass','maxInvMass') : CutResources(bram = 24 , dsp = 36, lut = 2000),
            ('minTransMass','maxTransMass') : CutResources(bram = 24 , dsp = 36, lut = 2000)
        })


    def getCollections(self, object):
        collections = {1: object.getParameter('collection1'), 2: object.getParameter('collection2')}
        for col in collections.values():
            self._InputTags += [col.getParameter("tag")]
        return collections




class SingleObjCond(Condition):
    """
    Class for to the L1GTSingleObjectCond 
    """
    def __init__(self):
        Condition.__init__(self)

        self._cut_aliases.update({ 
            'minPt' : 'pT_cut',
            'minEta' : 'minEta_cut',
            'maxEta' : 'maxEta_cut',
            'minPhi' : 'minPhi_cut',
            'maxPhi' : 'maxPhi_cut',
            'minZ0' : 'minZ0_cut',
            'maxZ0' : 'maxZ0_cut',
            'qual' : 'qual_cut',
            'iso' : 'iso_cut'})

    Label = "L1GTSingleObjectCond"
    Template = "single.template"


    def getCollections(self, object):
        self._InputTags += [object.getParameter("tag")]
        return {}



class QuadObjCond(Condition):
    """
    Class for to the L1GTQuadObjectCond 
    """
    def __init__(self):
        Condition.__init__(self)

        self._cut_aliases.update({
            'minPt' : 'pT_cuts',
            'minEta' : 'minEta_cuts',
            'maxEta' : 'maxEta_cuts',
            'minPhi' : 'minPhi_cuts',
            'maxPhi' : 'maxPhi_cuts',
            'minZ0' : 'minZ0_cuts',
            'maxZ0' : 'maxZ0_cuts',
            'qual' : 'qual_cuts',
            'iso' : 'iso_cuts',
            'os' : 'os_cuts',
            'ss' : 'ss_cuts'})

        self._HWConversionFunctions.update({
            'os'          : self.booltostring,
            'ss'          : self.booltostring
        })


    Label = "L1GTQuadObjectCond"
    Template = "quad.template"
    NumberOfCollections = 4
    NumberOfCorrelations = 6
    def getCollections(self, object):
        collections = {1: object.getParameter('collection1'), 2: object.getParameter('collection2'),3: object.getParameter('collection3'),4: object.getParameter('collection4')}
        for col in collections.values():
            self._InputTags += [col.getParameter("tag")]
        return collections


    def getCollections(self, object):
        collections = {1: object.getParameter('collection1'), 2: object.getParameter('collection2'),3: object.getParameter('collection3'),4: object.getParameter('collection4')}
        for col in collections.values():
            self._InputTags += [col.getParameter("tag")]

        return collections
    def getCorrelations(self, object):
        correlations = { 1: object.getParameter('delta12'), 2: object.getParameter('delta13'), 3 : object.getParameter('delta23'),4: object.getParameter('delta14'), 5: object.getParameter('delta24'), 6 : object.getParameter('delta34')}
        return correlations

class CutResources:
    def __init__(self,bram = 0,dsp = 0,lut = 0):
        self.bram = bram
        self.dsp = dsp
        self.lut = lut
    def addCutResources(self,cutResources):
        self.bram = self.bram + cutResources.bram
        self.dsp = self.dsp + cutResources.dsp
        self.lut = self.lut + cutResources.lut
    def printResources(self):
        print("bram:{}".format(self.bram))
        print("dsp:{}".format(self.dsp))
        print("lut:{}".format(self.lut))
        return 0

class DefineAlgoBits:
    def __init__(self):
        self.Assignment = {}
    def SetBit(self,Name,Algobit):
        self.Assignment[Name] = Algobit
    



class _Cut:
    def __init__(self):
        self.hwcut = []
        self.physcut = []
        self.enablecut = []
    def setCut(self,hwcut,physcut):
        self.hwcut.append(hwcut)
        self.physcut.append(physcut)
        self.enablecut.append(True)        
    def setNumberofvalues(self,value):
        self.hwcut = [0] * (value )
        self.physcut = [0] * (value)
        self.enablecut = [False] * (value)

    def setforBooleanCut(self,value):
        self.hwcut = [False] * (value)
    def setCutat(self,hwcut,physcut,position):
        self.hwcut[position -1] = hwcut
        self.physcut[position -1] = physcut
        self.enablecut[position -1] = True

    def __str__(self) -> str:
        pass


class LogicalFilter:
    def __init__(self,pathname,expression,modulenames):
        self.pathname = pathname 
        self.expression = expression
        self.modulenames = modulenames


class AlgorithmBlock:
    """
    Class that groups the algorithms in a way that all codependent conditions are grouped   
    """
    def __init__(self):
        self.ResourceUseage = CutResources(0,0,0)
        self.Modules = set()
        self.Paths  = set()
        self.Collections = set()
        self.LogicalPath = set()

    def addlogical(self,paths,modules):
        self.Modules.update(modules.modulenames)
        self.LogicalPath.add(paths)
    def checklogical(self,modules):
        """
        checks if a module in a logical combination is already present in modules
        """
        if(self.Modules.intersection(modules.modulenames) == set()):
            return 0
        else:
            return 1
    def Combineblocks(self,algoblock):
        self.ResourceUseage.addCutResources(algoblock.ResourceUseage) 
        self.Modules.update(algoblock.Modules) 
        self.Paths.update(algoblock.Paths)  
        self.LogicalPath.update(algoblock.LogicalPath)
        self.Collections.update(set(algoblock.Collections))

    def addCondition(self,knownfilterkey,knownfiltervalue):
        self.Modules.add(knownfilterkey)
        self.Paths.update(knownfiltervalue.Paths)
        self.ResourceUseage.addCutResources(knownfiltervalue.ResourceUseage)
        self.Collections.update(set(knownfiltervalue.InputObjects.values()))
    def checkCondition(self,knownfilterkey):
        """
        checks if module is already present in modules, needed to combine logical filters and conditions
        """
        if  knownfilterkey in self.Modules:
            return 1
        else:
            return 0 

class Algorithmsdict:
    def __init__(self):
        self.algoblocks = []

    def addLogicalFilters(self,logicalcombinations):

        for key, value in logicalcombinations.items():
            for algoblock in self.algoblocks:
                if algoblock.checklogical(key, value):
                    break
            else:
                newblock  = AlgorithmBlock()
                newblock.addlogical(key, value)
                self.algoblocks.append(newblock)
    def addConditions(self,conditions):
        for key in conditions.keys():
            for algoblock in self.algoblocks:
                if algoblock.checkCondition(key) == 1:
                    algoblock.addCondition(key,conditions[key])
                    break
            else:
                newblock  = AlgorithmBlock()
                newblock.addCondition(key,conditions[key])
                self.algoblocks.append(newblock)





    def popMaxalgoblock(self):

        if self.algoblocks == []:
            return 0
        else:
           brammax = max(item.ResourceUseage.bram for item in self.algoblocks)

           if brammax!=0 :
                for item, value in enumerate(self.algoblocks):
                    if value.ResourceUseage.bram == brammax:
                        return self.algoblocks.pop(item)

           dspmax = max(item.ResourceUseage.dsp for item in self.algoblocks)
           if dspmax!=0 :
                for item, value in enumerate(self.algoblocks):
                    if value.ResourceUseage.dsp == dspmax:
                        return self.algoblocks.pop(item)
           lutmax = max(item.ResourceUseage.lut for item in self.algoblocks)
           for item, value in enumerate(self.algoblocks):
                if value.ResourceUseage.lut == lutmax:
                    return self.algoblocks.pop(item)

            

class TripleObjCond(Condition):
    """
    Class for to the L1GTTripleObjectCond 
    """
    Label = "L1GTTripleObjectCond"
    Template = "triple.template"
    NumberOfCollections = 3
    NumberOfCorrelations = 3
    def __init__(self):
        Condition.__init__(self)
        
        self._HWConversionFunctions.update({
            'minDEta': l1tGTScales.to_hw_eta,
            'maxDEta': l1tGTScales.to_hw_eta,
            'minDPhi': l1tGTScales.to_hw_phi,
            'maxDPhi': l1tGTScales.to_hw_phi,
            'minDR': l1tGTScales.to_hw_dRSquared,
            'maxDR': l1tGTScales.to_hw_dRSquared,
            'minInvMass': l1tGTScales.to_hw_InvMassSqrDiv2,
            'maxInvMass': l1tGTScales.to_hw_InvMassSqrDiv2,
            'minTransMass': l1tGTScales.to_hw_TransMassSqrDiv2,
            'maxTransMass': l1tGTScales.to_hw_TransMassSqrDiv2,
            'os'          : self.booltostring,
            'ss'          : self.booltostring
        })

        
        self._cut_aliases.update({ 
            'minPt' : 'pT_cuts',
            'minEta' : 'minEta_cuts',
            'maxEta' : 'maxEta_cuts',
            'minPhi' : 'minPhi_cuts',
            'maxPhi' : 'maxPhi_cuts',
            'minZ0' : 'minZ0_cuts',
            'maxZ0' : 'maxZ0_cuts',
            'qual' : 'qual_cuts',
            'iso' : 'iso_cuts',
            'minDEta' : 'mindEta_cut',
            'maxDEta' : 'maxdEta_cut',
            'minDPhi' : 'mindPhi_cut',
            'maxDPhi' : 'maxdPhi_cut',
            'minDR' : 'mindRSquared_cut',
            'maxDR' : 'maxdRSquared_cut',
            'minInvMass' : 'mininvMassSqrDiv2_cut',
            'maxInvMass' : 'maxinvMassSqrDiv2_cut',
            'minTransMass' : 'mintransMassSqrDiv2_cut',
            'maxTransMass' : 'maxtransMassSqrDiv2_cut',
            'os' : 'os_cuts',
            'ss' : 'ss_cuts'
        })
        self.ResourcesperCut.update({
            ('minDEta','maxDEta') : CutResources(bram = 0 , dsp = 0, lut = 800),
            ('minDPhi','maxDPhi') : CutResources(bram = 0 , dsp = 0, lut = 800),
            ('minDR','maxDR') : CutResources(bram = 0 , dsp = 24, lut = 1800),
            ('minInvMass','maxInvMass') : CutResources(bram = 24 , dsp = 36, lut = 2000),
            ('minTransMass','maxTransMass') : CutResources(bram = 24 , dsp = 36, lut = 2000)
        })


    def getCollections(self, object):
        collections = {1: object.getParameter('collection1'), 2: object.getParameter('collection2'),3: object.getParameter('collection3')}
        for col in collections.values():
            self._InputTags += [col.getParameter("tag")]

        return collections
    def getCorrelations(self, object):
        correlations = { 1: object.getParameter('delta12'), 2: object.getParameter('delta23'), 3 : object.getParameter('delta13')}
        return correlations


