import jinja2
import L1Trigger.Phase2L1GT.VHDLWriter.Conditions
import os

def conditionwriter(name,attributes):
    templateLoader = jinja2.FileSystemLoader(searchpath=os.path.dirname(__file__))    # this has to be changed there seems to be a version of this thats called packageloader instead of filesystemloader
    templateEnv = jinja2.Environment(loader=templateLoader,trim_blocks=True,lstrip_blocks=True)
    template = templateEnv.get_template(attributes.Template,)
    if(attributes.Label == 'L1GTDoubleObjectCond'):
        outputText = template.render(condition_name = name, cuts = attributes.Cuts, objects_first = attributes.InputObjects.get(1),objects_second = attributes.InputObjects.get(2), algo_bit_name = name) 
    elif(attributes.Label == 'L1GTSingleObjectCond' ):
        outputText = template.render(condition_name = name, cuts = attributes.Cuts, object = attributes.InputObjects.get(1), algo_bit_name = name) 
    elif(attributes.Label == 'L1GTTripleObjectCond' ):
        outputText = template.render(condition_name = name, cuts = attributes.Cuts, objects_first = attributes.InputObjects.get(1),objects_second = attributes.InputObjects.get(2),objects_third = attributes.InputObjects.get(3), algo_bit_name = name) 
    elif(attributes.Label == 'L1GTQuadObjectCond' ):
        outputText = template.render(condition_name = name, cuts = attributes.Cuts, objects_first = attributes.InputObjects.get(1),objects_second = attributes.InputObjects.get(2),objects_third = attributes.InputObjects.get(3),objects_fourth = attributes.InputObjects.get(4), algo_bit_name = name) 

    else:
        print("Warning Condition not know to algounit writer, condition is of type{}".format(attributes.Label))
    return outputText

def algounitWriter(algobits,conditions,filts,logicalcomb,slrnumber):
    templateLoader = jinja2.FileSystemLoader(searchpath=os.path.dirname(__file__))    # this has to be changed there seems to be a version of this thats called packageloader instead of filesystemloader
    templateEnv = jinja2.Environment(loader=templateLoader,trim_blocks=True,lstrip_blocks=True)
    template = templateEnv.get_template("algounit.template",)
    outputText = template.render(algobits = algobits, Conditions = conditions,filtermodules = filts ,logicalcomb = logicalcomb,slrnumber = slrnumber)
    return outputText

def writeAlgounitToFile(filename,contents):
    f = open(filename,"w")
    f.write(contents)
    f.close()

    