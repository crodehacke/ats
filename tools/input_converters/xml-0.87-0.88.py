#!/usr/bin/env python
"""ATS input converter from 0.87 to 0.88"""

import sys, os
try:
    amanzi_xml = os.path.join(os.environ["AMANZI_SRC_DIR"], "tools","amanzi_xml")
except KeyError:
    pass
else:
    if amanzi_xml not in sys.path:
        sys.path.append(amanzi_xml)


from amanzi_xml.utils import search as asearch
from amanzi_xml.utils import io as aio
from amanzi_xml.utils import errors as aerrors
from amanzi_xml.common import parameter

def fixEvaluator(xml, name, newname):
    try:
        pd = asearch.childByNamePath(xml, "state/field evaluators/%s"%name)
    except aerrors.MissingXMLError:
        pass
    else:
        pd.setName(newname)

def linear_operator(xml):
    """Changes any instances of "linear operator" to "linear solver",
    which is now standard across all PKs."""

    pks = asearch.childByName(xml, "PKs")
    for pk in pks:
        try:
            lin_op = asearch.childByName(pk, "linear operator")
        except aerrors.MissingXMLError:
            pass
        else:
            lin_op.setName("linear solver")

def max_valid_change(xml):
    """Adds options for max valid change, which aren't required, but are strongly suggested."""
    pks = asearch.childByName(xml, "PKs")
    for pk in pks:
        pk_type = asearch.childByName(pk, "PK type")
        if pk_type.get('value') == 'permafrost flow':
            try:
                pk.getElement("max valid change in saturation in a time step [-]")
            except aerrors.MissingXMLError:
                pk.append(parameter.DoubleParameter("max valid change in saturation in a time step [-]", 0.1))

            try:
                pk.getElement("max valid change in ice saturation in a time step [-]")
            except aerrors.MissingXMLError:
                pk.append(parameter.DoubleParameter("max valid change in ice saturation in a time step [-]", 0.1))


def bad_spinup_longwave(xml):
    """One spinup file commonly used includes a longwave radiation 
    value that is totally wrong.  Not many runs actually used it.  
    Some runs even had a spec for it in their file, but didn't include 
    the necessary flag to use it.  So this just removes it to avoid 
    confusion."""
    evals = asearch.childByNamePath(xml, "state/field evaluators")
    try:
        lw = asearch.childByName(evals, "surface-incoming_longwave_radiation")
    except aerrors.MissingXMLError:
        pass
    else:
        try:
            filename = asearch.childByNamePath(lw, "function/domain/function/function-tabular/file")
        except aerrors.MissingXMLError:
            pass
        else:
            if "spinup-10yr.h5" in filename.get('value'):
                evals.pop("surface-incoming_longwave_radiation")


def sources(xml):
    """Can turn off derivative of source terms"""
    pks = asearch.childByName(xml, "PKs")
    for pk in pks:
        try:
            source_term = asearch.childByName(pk, "mass source key")
        except aerrors.MissingXMLError:
            pass
        else:
            source_term.setName('source key')
        try:
            source_term = asearch.childByName(pk, "energy source key")
        except aerrors.MissingXMLError:
            pass
        else:
            source_term.setName('source key')
            

        try:
            source_term = asearch.childByName(pk, "source term")
        except aerrors.MissingXMLError:
            pass
        else:
            if source_term.getValue():
                try:
                    source_is_diff = asearch.childByName(pk, "source term is differentiable")
                except aerrors.MissingXMLError:
                    pk.append(parameter.BoolParameter("source term is differentiable", True))
    
def snow_distribution(xml):
    for snow_dist_pk in asearch.generateElementByNamePath(xml, "PKs/snow distribution"):
        if snow_dist_pk.isElement("primary variable key") and \
             asearch.childByName(snow_dist_pk,"primary variable key").get("value") == "surface-precipitation_snow":
            asearch.childByName(snow_dist_pk,"primary variable key").set("value", "snow-precipitation")
        if snow_dist_pk.isElement("conserved quantity key") and \
             asearch.childByName(snow_dist_pk,"conserved quantity key").get("value") == "surface-precipitation_snow":
            asearch.childByName(snow_dist_pk,"conserved quantity key").set("value", "snow-precipitation")
        if snow_dist_pk.isElement("domain name") and \
           asearch.childByName(snow_dist_pk,"domain name").get("value") == "surface":
            asearch.childByName(snow_dist_pk,"domain name").set("value", "snow")
    
    for ssk in asearch.generateElementByNamePath(xml, "state/field evaluators/snow-conductivity"):
        if ssk.isElement("height key"):
            asearch.childByName(ssk, "height key").set("value", "snow-precipitation")

def end_time_units(xml):
    """yr --> y"""
    for end_time in asearch.generateElementByNamePath(xml, "cycle driver/end time units"):
        if end_time.get("value") == "yr":
            end_time.set("value", "y")


def surface_rel_perm_one(xml):
    """Add units, changed to pressure."""
    for surf_rel_perm in asearch.generateElementByNamePath(xml, "surface rel perm model"):
        pres_above = None
        if surf_rel_perm.isElement("unfrozen rel perm cutoff depth"):
            height_el = surf_rel_perm.pop("unfrozen rel perm cutoff height")
            pres_above = height_el.get("value") * 1000 * 10
        if surf_rel_perm.isElement("unfrozen rel pres cutoff pressure"):
            pres_el = surf_rel_perm.pop("unfrozen rel perm cutoff pressure")
            pres_above = pres_el.get("value")
        if surf_rel_perm.isElement("unfrozen rel pres cutoff pressure [Pa]"):
            continue
        else:
            if pres_above is not None:
                surf_rel_perm.append(parameter.DoubleParameter("unfrozen rel pres cutoff pressure [Pa]", pres_above)
            
            
            
def update(xml):
    linear_operator(xml)
    max_valid_change(xml)
    bad_spinup_longwave(xml)
    sources(xml)
    
    pks = asearch.childByName(xml, "PKs")
    for pk in pks:
        pk_type = asearch.childByName(pk, "PK type")
        if pk_type.get('value') == 'surface balance implicit':
            print('updating seb monolitic')
            import seb_monolithic_to_evals
            seb_monolithic_to_evals.update_seb(xml)

    fixEvaluator(xml, "surface-snow_skin_potential", "snow-skin_potential")
    fixEvaluator(xml, "surface-snow_conductivity", "snow-conductivity")
    snow_distribution(xml)
    end_time_units(xml)

    import verbose_object
    verbose_object.fixVerboseObject(xml)

    

if __name__ == "__main__":
    import argparse
    parser = argparse.ArgumentParser(description="Fix a number of changes from ATS input spec 0.86 to 0.88")
    parser.add_argument("infile", help="input filename")

    group = parser.add_mutually_exclusive_group()
    group.add_argument("-i", "--inplace", action="store_true", help="fix file in place")
    group.add_argument("-o", "--outfile", help="output filename")

    args = parser.parse_args()

    print "Converting file: %s"%args.infile
    xml = aio.fromFile(args.infile, True)
    update(xml)
    if args.inplace:
        aio.toFile(xml, args.infile)
    else:
        aio.toFile(xml, args.outfile)
    sys.exit(0)
    

    
