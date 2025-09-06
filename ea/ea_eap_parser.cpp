//
// Copyright 2019 Hans-Juergen Lange <hjl@simulated-universe.de>
//
// Permission is hereby granted, free of charge, to any person obtaining a copy of
// this software and associated documentation files (the “Software”), to deal in the
// Software without restriction, including without limitation the rights to use, copy,
// modify, merge, publish, distribute, sublicense, and/or sell copies of the Software,
// and to permit persons to whom the Software is furnished to do so, subject to the
// following conditions:
//
// The above copyright notice and this permission notice shall be included in all
// copies or substantial portions of the Software.
//
// THE SOFTWARE IS PROVIDED “AS IS”, WITHOUT WARRANTY OF ANY KIND, EXPRESS OR IMPLIED,
// INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY, FITNESS FOR A
// PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT
// HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION OF
// CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE
// OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.
#include <sys/types.h>
#include <sys/stat.h>

#ifdef __linux__
#include <unistd.h>
#endif

#if defined(WIN32) || defined(WIN64)
#include <direct.h>
#endif

#include <errno.h>
#include <limits.h>
#include <iostream>
#include <fstream>
#include <unordered_map>
#include "json.h"

#include <string>
#include <map>
#include <set>
#include <list>
#include "helper.h"
#include "variant.h"
#include "sqlrecordset.h"

#include "ea_eap_linux.h"

#include "ea_eap_parser.h"
#include "ea_eap_oledb.h"
#include "massociation.h"
#include "massociationend.h"
#include "mdependency.h"
#include "mgeneralization.h"
#include "mattribute.h"
#include "mparameter.h"
#include "moperation.h"
#include "mlifeline.h"
#include "mwebpagelifeline.h"
#include "mjslifeline.h"
#include "musecase.h"
#include "mclass.h"
#include "mmessage.h"
#include "mjsonmessage.h"
#include "msimmessage.h"
#include "msignal.h"
#include "msimsignal.h"
#include "mpackage.h"
#include "msimstatemachine.h"
#include "mstate.h"
#include "maction.h"
#include "mpseudostate.h"
#include "mfinalstate.h"
#include "mtransition.h"
#include "mevent.h"
#include "mpin.h"
#include "medge.h"
#include "mnode.h"
#include "minitialnode.h"
#include "mfinalnode.h"
#include "mactionnode.h"
#include "mdecisionnode.h"
#include "mmergenode.h"
#include "mjoinnode.h"
#include "mforknode.h"
#include "mactivity.h"
#include "mobject.h"
#include "mmodel.h"

#include "mrequirement.h"
#include "martifact.h"
#include "umldiagram.h"
#include "mnote.h"
#include "mnotelink.h"

#include "eaxref.h"

#include "cclassbase.h"
/*
 * This is the model we construct.
 */
static std::shared_ptr<MModel> model;
static std::map<std::string, tSQLRecordSet*>                         tables;
static std::map<std::string, std::unordered_map<long, std::string> > id_maps;

static std::map<std::string, eaXref*>                      xrefIdMap;
static std::multimap<std::string, eaXref*>                 xrefClientMap;
static std::multimap<std::string, eaXref*>                 xrefSupplierMap;

static std::list<std::shared_ptr<MClass>> gTypesToComplete;
static eVisibility getvisibility(const std::string& aVis) ;

static int objtable_parentid = -1;
static int objtable_packid   = -1;
static int objtable_nameid   = -1;

static std::string findnamespace(std::shared_ptr<MPackage> p) {
    std::string result;

    if (p->parent && p->parent->IsPackageBased()) {
        result = findnamespace(std::dynamic_pointer_cast<MPackage>(*p->parent));

        std::string ns = p->GetTaggedValue("namespace");
        if (!ns.empty()) {
            result += ns + "::";
        }
    }

    return result;
}

static std::string  findnamespace(std::shared_ptr<MClass> c) {
    std::string result;

    if (c->parent && c->parent->IsClassBased()) {
        result = findnamespace(std::dynamic_pointer_cast<MClass>(*c->parent));  // enclosed classes not supported yet.
    }
    else if (c->parent && c->parent->IsPackageBased()) {
        result = findnamespace(std::dynamic_pointer_cast<MPackage>(*c->parent));
    }
    return result;
}

//
//  find column id by its name. Automatic lowercase compare.
static int find_column_id(const std::string& name, tSQLRecordSet* aTable) {
    int retval = -1;

    for (size_t i = 0; i < aTable->FieldCount; i++) {
        if (helper::tolower(aTable->FieldNames[i]) == helper::tolower(name)) {
            retval = (int)i;
            break;
        }
    }

    return retval;
}
//
//  Create a map that connects some id to the ea_guid
static std::unordered_map<long, std::string> create_id_map(const std::string& aId, const std::string& aGuid, tSQLRecordSet* aTable) {
    std::unordered_map<long, std::string> retval;
    if (aTable->Error == 0) {
        int id_column   = find_column_id(aId, aTable);
        int guid_column = find_column_id(aGuid, aTable);

        for (size_t i = 0; i<aTable->Count; i++) {
            long        id   = aTable->Records[i].Fields[id_column];
            std::string guid = aTable->Records[i].Fields[guid_column];

            retval[id] = guid;
        }
    }
    return retval;
}
//
//  Loading all tables needed to parse the model.
static void ea_eap_load_tables(const char* aFileName) {
    tSQLRecordSet* result = nullptr;

    result = ea_eap_read_table(aFileName, "t_object");
    if (result != nullptr) {
        tables["objects"]  = result;
        id_maps["objects"] = create_id_map("object_id", "ea_guid", result);
        //
        // Collect the column ids.
        objtable_parentid = find_column_id("parentid", result);
        objtable_packid   = find_column_id("package_id", result);
        objtable_nameid = find_column_id("name", result);
    }
    result = ea_eap_read_table(aFileName, "t_stereotypes");
    if (result != nullptr) {
        tables["stereotypes"] = result;
    }
    result = ea_eap_read_table(aFileName, "t_objectproperties");
    if (result != nullptr) {
        tables["objectproperties"]  = result;
        id_maps["objectproperties"] = create_id_map("propertyid", "ea_guid", result);
    }
    result = ea_eap_read_table(aFileName, "t_package");
    if (result != nullptr) {
        tables["packages"]  = result;
        id_maps["packages"] = create_id_map("package_id", "ea_guid", result);
    }
    result = ea_eap_read_table(aFileName, "t_operation");
    if (result != nullptr) {
        tables["operations"]  = result;
        id_maps["operations"] = create_id_map("operationid", "ea_guid", result);
    }
    result = ea_eap_read_table(aFileName, "t_operationtag");
    if (result != nullptr) {
        tables["operationtags"] = result;
    }
    result = ea_eap_read_table(aFileName, "t_operationparams");
    if (result != nullptr) {
        tables["opparams"]  = result;
    }
    result = ea_eap_read_table(aFileName, "t_attribute");
    if (result != nullptr) {
        tables["attributes"]  = result;
        id_maps["attributes"] = create_id_map("id", "ea_guid", result);
    }
    result = ea_eap_read_table(aFileName, "t_attributetag");
    if (result != nullptr) {
        tables["attributetags"] = result;
    }
    result = ea_eap_read_table(aFileName, "t_connector");
    if (result != nullptr) {
        tables["connectors"]  = result;
        id_maps["connectors"] = create_id_map("connector_id", "ea_guid", result);
    }
    result = ea_eap_read_table(aFileName, "t_connectortag");
    if (result != nullptr) {
        tables["connectortags"] = result;
    }
    result = ea_eap_read_table(aFileName, "t_xref");
    if (result != nullptr) {
        tables["xref"]  = result;
    }
    result = ea_eap_read_table(aFileName, "t_diagram");
    if (result != nullptr) {
        tables["diagrams"] = result;
        id_maps["diagrams"] = create_id_map("diagram_id", "ea_guid", result);
    }
#ifndef __linux
    oledb_close();
#endif
}

static void fillbasicobject(const tRecord& aRecord, std::shared_ptr<MElement> aElement) {
    tSQLRecordSet* objtable = tables["objects"];
    //
    // Check the parent id. If zero the package is the parent.
    long        parent = aRecord.Fields[objtable_parentid];

    if (parent != 0) {
        auto & parentmap = id_maps["objects"];

        std::string parent_guid = parentmap[parent];
        aElement->parent = parent_guid;
    }
    else {
        //
        //  Fill the package back link
        auto & packagemap = id_maps["packages"];

        long        pack = aRecord.Fields[objtable_packid];
        std::string parent_guid = packagemap[pack];

        aElement->parent = parent_guid;
    }
    //
    //  File some attributes of the MElement.
    aElement->name = (std::string)aRecord.Fields[objtable_nameid];

    //
    // Attach the note as comment.
    long noteid = find_column_id("note", objtable);
    if (noteid != -1) {
        aElement->comment = (std::string)aRecord.Fields[noteid];
    }
    //
    //  Setup the position
    long position_id = find_column_id("tpos", objtable);
    long object_id   = find_column_id("object_id", objtable);

    if ((position_id != -1) && (object_id != -1)) {
        long position = aRecord.Fields[position_id];
        long object   = aRecord.Fields[object_id];

        aElement->mPosition = (((int64_t)position)<< 32) | (object & 0xffffffff);
    }
    //
    // Attach the note as comment.
    long scopeid = find_column_id("scope", objtable);
    if (scopeid != -1) {
        aElement->visibility = getvisibility((std::string)aRecord.Fields[scopeid]);
    }
    //
    //  Get the alias field.
    long aliasid = find_column_id("alias", objtable);
    if (aliasid != -1) {
        aElement->mAlias = (std::string)aRecord.Fields[aliasid];
    }
}

static void fillpack(const tRecord& aRecord, std::shared_ptr<MPackage> aPack) {
    //tSQLRecordSet*              objtable   = tables["objects"];
    //
    //  Fill the basics
    fillbasicobject(aRecord, aPack);
    long        pack = aRecord.Fields[objtable_packid];
    //
    //  Testing hack
    if (pack == 1) {
        model->Add(aPack);
    }
}

static void fillclass(const tRecord& aRecord, std::shared_ptr<MClass> aClass) {
    //
    //  Fill the basics
    fillbasicobject(aRecord, aClass);

    aClass->name = helper::trim((std::string)aRecord.Fields[objtable_nameid]);
    TypeNode temp = TypeNode::parse(aClass->name);

    if (temp.isCompositeType()) {
        gTypesToComplete.push_back(aClass);
    } else {
        gTypesToComplete.push_front(aClass);
    }
}

static eAggregation getaggregation(long aAggr) {
    eAggregation retval = eAggregation::aNone;

    switch (aAggr) {
    case 1:
        retval = eAggregation::aShared;
        break;
    case 2:
        retval = eAggregation::aComposition;
        break;
    default:
        break;
    }
    return retval;
}

static eVisibility getvisibility(const std::string& aVis) {
    std::string vis = helper::tolower(aVis);

    if (vis == "private") {
        return vPrivate;
    } else if (vis == "public") {
        return vPublic;
    } else if (vis == "package") {
        return vPackage;
    } else if (vis == "protected") {
        return vProtected;
    }
    return vPublic;
}


static void fillassocend(const tRecord& aRecord, std::shared_ptr<MAssociationEnd> aEnd, std::string aType) {
    tSQLRecordSet* con_table = tables["connectors"];
    tSQLRecordSet* obj_table = tables["objects"];

    if ((con_table != nullptr) && (obj_table != nullptr) && (aEnd != nullptr) && ((aType == "source") || (aType == "dest"))) {
        int classifier_id;
        int role_id;

        if (aType == "source") {
            classifier_id = find_column_id("start_object_id", con_table);
        } else {
            classifier_id = find_column_id("end_object_id", con_table);
        }

        role_id        = find_column_id(aType+"role", con_table);
        //
        //  Process own.
        int access_id       = find_column_id(aType+"access", con_table);
        int is_aggr_id      = find_column_id(aType+"isaggregate", con_table);
        int is_ordered_id   = find_column_id(aType+"isordered", con_table);
        int qualifier_id    = find_column_id(aType+"qualifier", con_table);
        int is_navigable_id = find_column_id(aType+"isnavigable", con_table);
        int stereotype_id   = find_column_id(aType+"stereotype", con_table);
        int style_id        = find_column_id(aType+"style", con_table);
        int multiplicity_id = find_column_id(aType+"card", con_table);

        if ((classifier_id != -1) &&
            (access_id != -1) &&
            (role_id != -1) &&
            (is_aggr_id != -1) &&
            (is_ordered_id != -1) &&
            (qualifier_id != -1) &&
            (is_navigable_id != -1) &&
            (stereotype_id != -1) &&
            (style_id != -1) &&
            (multiplicity_id != -1)) {

            auto & id_map    = id_maps["objects"];

            aEnd->Classifier = id_map[aRecord.Fields[classifier_id]];

            aEnd->visibility    = getvisibility(aRecord.Fields[access_id]);
            aEnd->name          = (std::string)aRecord.Fields[role_id];
            aEnd->Aggregation   = getaggregation(aRecord.Fields[is_aggr_id]);
            aEnd->mQualifier    = (std::string)aRecord.Fields[qualifier_id];
            aEnd->Navigable     = ((bool)(aRecord.Fields[is_navigable_id]) == true)?eNavigable::yes:eNavigable::no;
            aEnd->Multiplicity  = (std::string)aRecord.Fields[multiplicity_id];
        }
    } else {

    }
}

static void fillassoc(const tRecord& aRecord, std::shared_ptr<MAssociation> aAssoc) {
    tSQLRecordSet* con_table = tables["connectors"];
    tSQLRecordSet* obj_table = tables["objects"];
    //
    //
    if ((con_table != nullptr) && (obj_table != nullptr)) {
        int direction_id = find_column_id("direction", con_table);
        int src_id       = find_column_id("start_object_id", con_table);
        int dst_id       = find_column_id("end_object_id", con_table);
        int name_id      = find_column_id("name", con_table);

        if ((direction_id != -1) && (src_id != -1) && (dst_id != -1) && (name_id != -1)) {
            std::string                 direction = helper::tolower(aRecord.Fields[direction_id]);
//            long                        src       = aRecord.Fields[src_id];   // This is the classifier the assoc-end is pointing to.
//            long                        dst       = aRecord.Fields[dst_id];   // This is the classifier the assoc-end is pointing to.

            aAssoc->name       = (std::string)aRecord.Fields[name_id];
            aAssoc->visibility = vPublic;

            //
            //  Create the ends
            auto own   = MAssociationEnd::construct(aAssoc->id + "-own", aAssoc);

            fillassocend(aRecord, own, "dest");
            aAssoc->AddEnd(own);
            model->Add(own);

            auto other = MAssociationEnd::construct(aAssoc->id + "-other", aAssoc);

            fillassocend(aRecord, other, "source");
            aAssoc->AddEnd(other);
            model->Add(other);
        }
    } else {

    }
}

static void fillgeneralization(const tRecord& aRecord, std::shared_ptr<MGeneralization> aGeneralization) {
    tSQLRecordSet* con_table = tables["connectors"];
    tSQLRecordSet* obj_table = tables["objects"];
    //
    //
    if ((con_table != nullptr) && (obj_table != nullptr)) {
        int direction_id = find_column_id("direction", con_table);
        int src_id       = find_column_id("start_object_id", con_table);
        int dst_id       = find_column_id("end_object_id", con_table);
        int name_id      = find_column_id("name", con_table);

        if ((direction_id != -1) && (src_id != -1) && (dst_id != -1) && (name_id != -1)) {
            auto & id_map    = id_maps["objects"];
            std::string                 direction = helper::tolower(aRecord.Fields[direction_id]);
            long                        src       = aRecord.Fields[src_id];
            long                        dst       = aRecord.Fields[dst_id];

            aGeneralization->name       = (std::string)aRecord.Fields[name_id];
            aGeneralization->visibility = vPublic;

            if (direction == "source -> destination") {
                aGeneralization->base    = id_map[dst];
                aGeneralization->derived = id_map[src];
            } else if (direction == "destination -> source") {
                aGeneralization->base    = id_map[src];
                aGeneralization->derived = id_map[dst];
            } else {
                std::cerr << "Could not find direction on connector " << aGeneralization->id << std::endl;
            }
        }
    } else {

    }
}

static void filldependency(const tRecord& aRecord, std::shared_ptr<MDependency> aDependency) {
    tSQLRecordSet* con_table = tables["connectors"];
    tSQLRecordSet* obj_table = tables["objects"];
    //
    //
    if ((con_table != nullptr) && (obj_table != nullptr)) {
        int direction_id = find_column_id("direction", con_table);
        int src_id       = find_column_id("start_object_id", con_table);
        int dst_id       = find_column_id("end_object_id", con_table);
        int name_id      = find_column_id("name", con_table);
        int styleex_id   = find_column_id("styleex", con_table);

        if ((direction_id != -1) && (src_id != -1) && (dst_id != -1) && (name_id != -1) && (styleex_id != -1)
            ) {
            auto & id_map    = id_maps["objects"];
            std::string                 direction = helper::tolower(aRecord.Fields[direction_id]);
            long                        src       = aRecord.Fields[src_id];
            long                        dst       = aRecord.Fields[dst_id];
            std::string                 style     = aRecord.Fields[styleex_id];

            aDependency->name       = (std::string)aRecord.Fields[name_id];
            aDependency->visibility = vPublic;
            //
            //  process styleex if existing.
            std::string feature_ref;
            std::string src_ref    = id_map[src];

            if (!style.empty()) {
                size_t startpos = style.find('=');

                if ((startpos != std::string::npos) && (startpos+1 < style.size())) {
                    size_t endpos = style.find('}');

                    if (endpos != std::string::npos) {
                        endpos = endpos - startpos;
                    }
                    std::string wheretoput = style.substr(0, startpos);
                    
                    if (wheretoput == "LFSP") {
                        src_ref = style.substr(startpos+1, endpos);
                    }
                }
            }
            if (direction == "source -> destination") {
                aDependency->src    = src_ref;
                aDependency->target = id_map[dst];
            } else if (direction == "destination -> source") {
                aDependency->target = src_ref;
                aDependency->src    = id_map[dst];
            } else {
                std::cerr << "Could not find direction on connector " << aDependency->id << std::endl;
            }
        }
    } else {

    }
}


static void fillnotelink(const tRecord& aRecord, std::shared_ptr<MNoteLink> aNoteLink) {
    tSQLRecordSet* con_table = tables["connectors"];
    tSQLRecordSet* obj_table = tables["objects"];
    //
    //
    if ((con_table != nullptr) && (obj_table != nullptr)) {
        int direction_id = find_column_id("direction", con_table);
        int src_id = find_column_id("start_object_id", con_table);
        int dst_id = find_column_id("end_object_id", con_table);
        int name_id = find_column_id("name", con_table);

        if ((direction_id != -1) && (src_id != -1) && (dst_id != -1) && (name_id != -1)) {
            auto & id_map = id_maps["objects"];
            std::string                 direction = helper::tolower(aRecord.Fields[direction_id]);
            long                        src = aRecord.Fields[src_id];
            long                        dst = aRecord.Fields[dst_id];

            aNoteLink->name = (std::string)aRecord.Fields[name_id];
            aNoteLink->visibility = vPublic;

            if (direction == "source -> destination") {
                aNoteLink->src = id_map[src];
                aNoteLink->target = id_map[dst];
            }
            else if (direction == "destination -> source") {
                aNoteLink->target = id_map[src];
                aNoteLink->src = id_map[dst];
            }
            else {
                std::cerr << "Could not find direction on connector " << aNoteLink->id << std::endl;
            }
        }
    }
    else {

    }
}


static void fillrequirement(const tRecord& aRecord, std::shared_ptr<MRequirement> aRequirement) {
    //tSQLRecordSet* objtable = tables["objects"];
    //
    //  Fill the basics
    fillbasicobject(aRecord, aRequirement);
}
static void fillartifact(const tRecord& aRecord, std::shared_ptr<MArtifact> aArtifact) {
    tSQLRecordSet* objtable = tables["objects"];
    //
    //  Fill the basics
    fillbasicobject(aRecord, aArtifact);
    //
    //
    long styleid = find_column_id("style", objtable);

    std::string style = (std::string)aRecord.Fields[styleid];
    //
    //  

}
static void filldiagram(const tRecord& aRecord, std::shared_ptr<UmlDiagram> aDiagram) {
    tSQLRecordSet* objtable = tables["objects"];
    //
    //  Fill the basics
    fillbasicobject(aRecord, aDiagram);
    //
    //
    long diagramid = find_column_id("pdata1", objtable);

    if (diagramid != -1) {
        long diagram = aRecord.Fields[diagramid];

        auto & diagrammap = id_maps["diagrams"];

        aDiagram->mReference = diagrammap[diagram];
    }
    //
    //  Setup the position
    long position_id = find_column_id("tpos", objtable);
    long object_id = find_column_id("object_id", objtable);

    if ((position_id != -1) && (object_id != -1)) {
        long position = aRecord.Fields[position_id];
        long object = aRecord.Fields[object_id];

        aDiagram->mPosition = (((int64_t)position) << 32) | (object & 0xffffffff);
    }
}
//
//  Load the stereotypes
static void ea_eap_load_stereotypes(std::shared_ptr<MModel> aModel) {
    //
    //  Scan the stereotype table.
    tSQLRecordSet* table = tables["stereotypes"];
    int name_id = find_column_id("stereotype", table);
    int guid_id = find_column_id("ea_guid", table);
    int etype_id = find_column_id("appliesto", table);

    if ((name_id != -1) && (guid_id != -1)) {
        for (auto & s : table->Records) {
            auto newstereotype = new MStereotype(s.Fields[name_id], s.Fields[guid_id], s.Fields[etype_id]);
            aModel->Add(newstereotype->sharedthis<MStereotype>());
        }
    }
    //
    //  Scan the objects table.
    table = tables["objects"];

    if (table != nullptr) {
        int stereotype_id = find_column_id("stereotype", table);
        guid_id           = find_column_id("ea_guid", table);

        for (auto & s : table->Records) {
            if (helper::tolower(s.Fields[stereotype_id]) == "stereotype") {
                auto newstereotype = new MStereotype(s.Fields[objtable_nameid], s.Fields[guid_id], "");
                aModel->Add(newstereotype->sharedthis<MStereotype>());
            }
        }
    }
}
//
//  Load the diagrams
static void ea_eap_load_diagrams(std::shared_ptr<MModel> aModel) {
    //
    //  Scan the stereotype table.
    tSQLRecordSet* table = tables["diagrams"];

    int diagram_id = find_column_id("diagram_id", table);
    int name_id = find_column_id("name", table);
    int guid_id = find_column_id("ea_guid", table);
    int package_id = find_column_id("package_id", table);
    int parent_id  = find_column_id("parentid", table);

    if ((name_id != -1) && (guid_id != -1) && (diagram_id != -1) && (package_id != -1) &&
        (parent_id != -1)) {
        for (auto& d : table->Records) {
            auto diagram = UmlDiagram::construct(d.Fields[guid_id], nullptr);

            if (diagram) {
                long parent = d.Fields[parent_id];

                if (parent == 0) {
                    //
                    //  Fill the package back link
                    auto & packagemap = id_maps["packages"];

                    long        pack = d.Fields[package_id];
                    std::string parent_guid = packagemap[pack];

                    diagram->parent = parent_guid;
                }
                else {
                    std::string pid = id_maps["objects"][parent];

                    diagram->parent = pid;
                }
            
                if (diagram->parent) {
                    diagram->parent->Add(diagram);
                }
                diagram->name       = (std::string)d.Fields[name_id];
                diagram->mReference = diagram->id;
                //
                //  Setup the position
                long position_id = find_column_id("tpos", table);

                if ((position_id != -1) && (diagram_id != -1)) {
                    long position = d.Fields[position_id];
                    long object   = d.Fields[diagram_id];

                    diagram->mPosition = (((int64_t)position) << 32) | (object & 0xffffffff);
                }
                //
                //
                int stereotype_id = find_column_id("stereotype", table);

                if (stereotype_id != -1) {
                    std::string st = (std::string)d.Fields[stereotype_id];

                    if (!st.empty()) {
                        auto s = aModel->StereotypeByName(st, "");

                        if (!s) {

                            auto new_s = new MStereotype(st, st + "-diagram");
                            aModel->Add(new_s->sharedthis<MStereotype>());
                            s = new_s->sharedthis<MStereotype>();
                        }

                        if (s) {
                            diagram->stereotypes.insert(std::pair<std::string, std::shared_ptr<MStereotype>>(helper::tolower(st),s));
                        }
                    }
                }
            }
        }
    }
}
//
//  Load packages
static void ea_eap_load_roots(std::shared_ptr<MModel> aModel) {
    tSQLRecordSet* table = tables["packages"];

    if (table != nullptr) {
        int package_id = find_column_id("package_id", table);
        int name_id = find_column_id("name", table);
        int guid_id = find_column_id("ea_guid", table);
        int parent_id = find_column_id("parent_id", table);

        if ((package_id != -1) && (guid_id != -1) && (name_id != -1) && (parent_id != -1)) {
            long         parent;

            for (auto& s : table->Records) {
                parent = s.Fields[parent_id];

                if (parent == 0) {
                    std::string  id = (std::string)s.Fields[guid_id];

                    auto mpack = MPackage::construct(id, "UMLModel");

                    mpack->name = (std::string)s.Fields[name_id];
                    aModel->Add(mpack);
                }
            }
        }
    }
}

//
//  Load packages
static void ea_eap_load_objects(std::shared_ptr<MModel> aModel) {
    tSQLRecordSet* table = tables["objects"];
    std::set<std::string> unknown;

    if (table != nullptr) {
        int stereotype_id = find_column_id("stereotype", table);
        int guid_id       = find_column_id("ea_guid", table);
        int objtype_id    = find_column_id("object_type", table);

        if ((stereotype_id != -1) && (guid_id != -1) && (objtype_id != -1) ) {
            std::string  type;
            std::string  id;
            std::string  stereotype;
            std::shared_ptr<MStereotype> st;

            for (auto & s : table->Records) {
                type       = helper::tolower(s.Fields[objtype_id]);
                id         = (std::string)s.Fields[guid_id];
                stereotype = (std::string)s.Fields[stereotype_id];

                if (!stereotype.empty()) {
                    st         = aModel->StereotypeByName(stereotype, type);
                } else {
                    st         = std::shared_ptr<MStereotype>();
                }
                //
                //  Different processing for different types.
                if (type == "package") {

                    std::shared_ptr<MPackage>  pack;
                    if (st) {
                        pack = MPackage::construct(id, st);
                    } else {
                        pack = MPackage::construct(id, stereotype);
                    }

                    if (pack) {
                        fillpack(s, pack);
                    }
                } else if (type == "class") {
                    if (helper::tolower(stereotype) != "stereotype") {
                        std::shared_ptr<MClass> c;
                        if (st) {
                            c = MClass::construct(id, st);
                        } else {
                            c = MClass::construct(id, stereotype);
                        }
                    
                        if (c) {
                            fillclass(s, c);
                            //
                            //  connect the parameters.
                            auto range = xrefClientMap.equal_range(id);
                            for (auto & it = range.first; it != range.second; ++it) {
                                if (it->second->mDescription.mStruct.size() > 0) {
                                    auto fields = it->second->mDescription.mStruct.begin();
                                    std::string guid;
                                    std::string name;
                                    std::string t;
                                    std::string pos;
                                    std::string etype;

                                    if (fields->mName == "@ELEMENT") {
                                        //
                                        //  Setup the object parameter.
                                        for (; fields != it->second->mDescription.mStruct.end(); ++fields) {
                                            if (fields->mName == "GUID") {
                                                guid = fields->mValue;
                                            } else if (fields->mName == "Name") {
                                                name = fields->mValue;
                                            } else if (fields->mName == "Type") {
                                                t = fields->mValue;
                                            } else if (fields->mName == "Pos") {
                                                pos = fields->mValue;
                                            } else if (fields->mName == "ParameteredElementType") {
                                                etype = fields->mValue;
                                            } else {
                                            }
                                        }
                                        auto templateparameter = new MParameter(guid, c);

                                        templateparameter->name = name;
                                        templateparameter->ClassifierName = etype;
                                        templateparameter->mPosition = atoi(pos.c_str());

                                        c->mClassParameter[atoi(pos.c_str())] = templateparameter->sharedthis<MParameter>();
                                    } else if ((fields->mName == "@PROP") && (fields->mValue == "@NAME=isFinalSpecialization@ENDNAME")) {
                                        //
                                        //  Final flag value found;
                                        for (; fields != it->second->mDescription.mStruct.end(); ++fields) {
                                            if (fields->mName == "@VALU") {
                                                c->mIsFinal = std::atoi(fields->mValue.c_str());
                                            } else {
                                            }
                                        }
                                    }
                                }
                            }
                        }
                    }
                } else if (type == "signal") {
                    if (stereotype.empty()) {
                        stereotype = "signal";
                    }
                    std::shared_ptr<MClass> c;
                    if (st) {
                        c = MClass::construct(id, st);
                    } else {
                        c = MClass::construct(id, stereotype);
                    }
                    
                    if (c) {
                        fillclass(s, c);
                    }
                } else if (type == "requirement") {
                    auto r = MRequirement::construct(id, nullptr);

                    if (r) {
                        fillrequirement(s, r);
                        r->kind = stereotype;
                    }
                }
                else  if (type == "artifact") {
                    auto a = MArtifact::construct(id, nullptr);
                    if (a) {
                        fillartifact(s, a);
                        a->mType = stereotype;
                    }
                }
                else if (type == "umldiagram") {
                    auto d = UmlDiagram::construct(id, nullptr);

                    if (d) {
                        filldiagram(s, d);
                    }
                }
                else if (type == "note") {
                    auto n = MNote::construct(id, nullptr);

                    if (n) {
                        fillbasicobject(s, n);
                    }
                }
                else if (type == "primitivetype") {
                    auto c = MClass::construct(id, "PrimitiveType", nullptr);

                    if (c) {
                        fillclass(s, c);
                    }
                }
                else if (type == "enumeration") {
                    auto c = MClass::construct(id, "Enumeration", nullptr);

                    if (c) {
                        fillclass(s, c);
                    }
                } else if (type == "interface") {
                    auto c = MClass::construct(id, "interface", nullptr);

                    if (c) {
                        fillclass(s, c);
                    }
                }
                else {
                    if (unknown.find(type) == unknown.end()) {
                        unknown.insert(type);
                        std::cerr << "Unknown object type : " << type << std::endl;
                    }
                    //
                    //  To avoid confusion here.
                    //  The MElement constructor always collects the elements into its maps.
                    //  So the pointer is not lost.
                    MElement* e = new MElement(id, nullptr);
                    static_cast<void>(e);
                }
            }
            tSQLRecordSet* props = tables["objectproperties"];

            if (props != nullptr) {
                int objid = find_column_id("object_id", props);
                int propid = find_column_id("property", props);
                int valueid = find_column_id("value", props);

                if ((objid != -1) && (propid != -1) && (valueid != -1)) {
                    long oid;
                    std::string name;
                    std::string value;
                    auto & id_map = id_maps["objects"];

                    for (auto& p : props->Records) {
                        oid = p.Fields[objid];
                        name = (std::string)p.Fields[propid];
                        value = (std::string)p.Fields[valueid];

                        id = id_map[oid];
                        auto e = MElement::Instances.find(id);

                        if ((e != MElement::Instances.end()) && (e->second != nullptr)) {
                            if (!e->second->AddTag(name, value)) {
                                std::cerr << "Cannot add tagged value " << name << " to object " << e->second->name << ". Probably double defined.\n";
                            }
                        }
                            
                    }
                }
            }
        }
    } else {

    }
}
//
//  Load operations
static void ea_eap_load_operations(std::shared_ptr<MModel> aModel) {
    tSQLRecordSet*              table  = tables["operations"];
    auto & id_map = id_maps["objects"];

    if (table != nullptr) {
        int guid_id       = find_column_id("ea_guid", table);
        int object_id     = find_column_id("object_id", table);
        int optype_id     = find_column_id("type", table);
        int name_id       = find_column_id("name", table);
        int scope_id      = find_column_id("scope", table);
        int static_id     = find_column_id("isstatic", table);
        int abstract_id   = find_column_id("abstract", table);
        int position_id   = find_column_id("pos", table);
        int const_id      = find_column_id("const", table);
        int pure_id       = find_column_id("pure", table);
        int throws_id     = find_column_id("throws", table);
        int classifier_id = find_column_id("classifier", table);
        int code_id       = find_column_id("code", table);
        int isquery_id    = find_column_id("isquery", table);
        int notes_id      = find_column_id("notes", table);
        int stereotype_id = find_column_id("stereotype", table);


        if ((object_id != -1) && (guid_id != -1) && (optype_id != -1) && (name_id != -1) &&
            (scope_id != -1) && (static_id != -1) && (abstract_id != -1) && (position_id != -1) &&
            (const_id != -1) && (pure_id != -1) && (throws_id != -1) && (classifier_id != -1) &&
            (code_id != -1) && (isquery_id != -1) && (notes_id != -1) && (stereotype_id != -1)
            ) {
            std::string  id;
            long         object;
            std::string  optype;
            std::string  name;
            std::string  scope;
            bool         isstatic;
            bool         isabstract;
            long         position;
            bool         isconst;
            bool         ispure;
            std::string  throws;
            std::string  classifier;
            std::string  code;
            bool         isquery;
            std::string  notes;
            std::string  stereotype;

            for (auto & s : table->Records) {
                id         = (std::string)s.Fields[guid_id];
                optype     = std::string(s.Fields[optype_id]);
                object     = s.Fields[object_id];
                name       = (std::string)s.Fields[name_id];
                scope      = helper::tolower(s.Fields[scope_id]);
                isstatic   = s.Fields[static_id];
                isabstract = s.Fields[abstract_id];
                position   = s.Fields[position_id];
                isconst    = s.Fields[const_id];
                ispure     = s.Fields[pure_id];
                throws     = (std::string)s.Fields[throws_id];
                classifier = (std::string)s.Fields[classifier_id];
                code       = (std::string)s.Fields[code_id];
                isquery    = s.Fields[isquery_id];
                notes      = (std::string)s.Fields[notes_id];
                stereotype = (std::string)s.Fields[stereotype_id];

                std::string guid = id_map[object];

                auto pi = MElement::Instances.find(guid);

                std::shared_ptr<MElement> parent;
                
                if (pi != MElement::Instances.end()) {
                    parent = pi->second;
                }
                auto op = MOperation::construct(id, parent);

                op->parent         = guid;
                op->isAbstract     = isabstract;
                op->isQuery        = isquery;
                op->isStatic       = isstatic;
                op->isPure         = ispure;
                op->hasConstReturn = isconst;
                op->name           = name;
                op->visibility     = getvisibility(scope);
                op->mPosition      = position;
                op->Specification  = code;
                op->comment        = notes;

                std::shared_ptr<MStereotype> st;
                if (!stereotype.empty()) {
                    st         = aModel->StereotypeByName(stereotype);
                    op->stereotypes.emplace(stereotype, st);
                } else {
                    st         = std::shared_ptr<MStereotype>();
                }

                if (parent->IsClassBased()) {
                    auto c = std::dynamic_pointer_cast<MClass>(parent);

                    c->Operation.emplace_back(op);
                }
                if (!optype.empty()) {
                    auto ret = MParameter::construct(id + "-return", op);

                    ret->parent         = id;
                    ret->ClassifierName = optype;
                    ret->Direction      = "return";
                    ret->isReadOnly     = isconst;

                    op->Parameter.emplace_back(ret);
                }
                auto x = xrefClientMap.find(id);

                while ((x != xrefClientMap.end()) && (x->first == id)) {
                    if (helper::tolower(x->second->mBehavior) == "raisedexception") {
                        op->mException = x->second->mDescription.mStruct.begin()->mValue;
                    }
                    ++x;
                }

            }
            tSQLRecordSet* props = tables["operationtags"];

            if (props != nullptr) {
                int objid = find_column_id("elementid", props);
                int propid = find_column_id("property", props);
                int valueid = find_column_id("value", props);

                if ((objid != -1) && (propid != -1) && (valueid != -1)) {
                    long oid;
                    std::string name;
                    std::string value;
                    auto & id_map = id_maps["operations"];

                    for (auto& p : props->Records) {
                        oid = p.Fields[objid];
                        name = (std::string)p.Fields[propid];
                        value = (std::string)p.Fields[valueid];

                        id = id_map[oid];
                        auto e = MElement::Instances.find(id);

                        if ((e != MElement::Instances.end()) && (e->second != nullptr)) {
                            if (!e->second->AddTag(name, value)) {
                                std::cerr << "Cannot add tagged value " << name << " to object " << e->second->name << ". Probably double defined.\n";
                            }
                        }

                    }
                }
            }
        }
    } else {

    }

}
//
//  Load attributes
static void ea_eap_load_attributes(std::shared_ptr<MModel> aModel) {
    tSQLRecordSet*              table  = tables["attributes"];
    auto & id_map = id_maps["objects"];

    if (table != nullptr) {
        int guid_id       = find_column_id("ea_guid", table);
        int object_id     = find_column_id("object_id", table);
        int attrtype_id   = find_column_id("type", table);
        int name_id       = find_column_id("name", table);
        int scope_id      = find_column_id("scope", table);
        int static_id     = find_column_id("isstatic", table);
        int position_id   = find_column_id("pos", table);
        int const_id      = find_column_id("const", table);
        int classifier_id = find_column_id("classifier", table);
        int lower_id      = find_column_id("lowerbound", table);
        int upper_id      = find_column_id("upperbound", table);
        int default_id    = find_column_id("default", table);
        int notes_id      = find_column_id("notes", table);


        if ((object_id != -1) && (guid_id != -1) && (attrtype_id != -1) && (name_id != -1) &&
            (scope_id != -1) && (static_id != -1) && (default_id != -1) && (position_id != -1) &&
            (const_id != -1) && (upper_id != -1) && (lower_id != -1) && (classifier_id != -1) && (notes_id != -1)) {
            std::string  id;
            long         object;
            std::string  attrtype;
            std::string  name;
            std::string  scope;
            bool         isstatic;
            long         position;
            bool         isconst;
            std::string  classifier;
            std::string  lower;
            std::string  upper;
            std::string  defaultValue;
            std::string  notes;

            for (auto & s : table->Records) {
                object     = s.Fields[object_id];

                std::string guid = id_map[object];

                auto f = MElement::Instances.find(guid);
                if (f != MElement::Instances.end()) {
                    std::shared_ptr<MElement>  parent = f->second;

                    if (parent) {
                        id           = (std::string)s.Fields[guid_id];
                        attrtype     = std::string(s.Fields[attrtype_id]);
                        name         = (std::string)s.Fields[name_id];
                        scope        = helper::tolower(s.Fields[scope_id]);
                        isstatic     = s.Fields[static_id];
                        position     = s.Fields[position_id];
                        isconst      = s.Fields[const_id];
                        classifier   = (std::string)s.Fields[classifier_id];
                        lower        = (std::string)s.Fields[lower_id];
                        upper        = (std::string)s.Fields[upper_id];
                        defaultValue = (std::string)s.Fields[default_id];
                        notes        = (std::string)s.Fields[notes_id];

                        auto attr = MAttribute::construct(id, parent);

                        if (attr) {
                            attr->ClassifierName = attrtype;
                            attr->name           = name;
                            attr->visibility     = getvisibility(scope);
                            attr->isStatic       = isstatic;
                            attr->isReadOnly     = isconst;
                            attr->mPosition      = position;
                            attr->defaultValue   = defaultValue;
                            attr->comment        = notes;

                            if (!classifier.empty()) {
                                long cl = std::atol(classifier.c_str());

                                if (cl != 0) {
                                    attr->Classifier  = id_map[cl];
                                }
                            }

                            if (!lower.empty()) {
                                attr->Multiplicity = lower;
                            } else {
                                attr->Multiplicity = "1";
                            }
                            if ((!upper.empty()) && (upper != lower)) {
                                if (!attr->Multiplicity.empty())   {
                                    attr->Multiplicity += "..";
                                }
                                attr->Multiplicity     += upper;
                            }

                            if (parent->IsClassBased()) {
                                auto c = std::dynamic_pointer_cast<MClass>(parent);

                                c->AddAttribute(attr);
                            }

                        }
                    }

                }
            }
            tSQLRecordSet* props = tables["attributetags"];

            if (props != nullptr) {
                int objid = find_column_id("object_id", props);
                int propid = find_column_id("property", props);
                int valueid = find_column_id("value", props);

                if ((objid != -1) && (propid != -1) && (valueid != -1)) {
                    long oid;
                    std::string name;
                    std::string value;
                    auto & id_map = id_maps["attributes"];

                    for (auto& p : props->Records) {
                        oid = p.Fields[objid];
                        name = (std::string)p.Fields[propid];
                        value = (std::string)p.Fields[valueid];

                        id = id_map[oid];
                        auto e = MElement::Instances.find(id);

                        if ((e != MElement::Instances.end()) && (e->second != nullptr)) {
                            if (!e->second->AddTag(name, value)) {
                                std::cerr << "Cannot add tagged value " << name << " to object " << e->second->name << ". Probably double defined.\n";
                            }
                        }

                    }
                }
            }

        }

    }
}
//
//  Load OperationParameters
static void ea_eap_load_opparams(std::shared_ptr<MModel> aModel) {
    tSQLRecordSet*              table  = tables["opparams"];
    auto & id_map = id_maps["operations"];
    auto & object_id_map = id_maps["objects"];

    if (table != nullptr) {
        int guid_id       = find_column_id("ea_guid", table);
        int operation_id  = find_column_id("operationid", table);
        int paratype_id   = find_column_id("type", table);
        int name_id       = find_column_id("name", table);
        int default_id    = find_column_id("default", table);
        int position_id   = find_column_id("pos", table);
        int const_id      = find_column_id("const", table);
        int kind_id       = find_column_id("kind", table);
        int classifier_id = find_column_id("classifier", table);
        int notes_id      = find_column_id("notes", table);


        if ((operation_id != -1) && (guid_id != -1) && (paratype_id != -1) && (name_id != -1) &&
            (default_id != -1) && (position_id != -1) &&
            (const_id != -1) && (kind_id != -1) && (classifier_id != -1) && (notes_id != -1)
            ) {
            std::string  id;
            long         operation;
            std::string  paratype;
            std::string  name;
            std::string  defaultValue;
            long         position;
            bool         isconst;
            std::string  kind;
            std::string  classifier;
            std::string  notes; 

            for (auto & s : table->Records) {
                id           = (std::string)s.Fields[guid_id];
                paratype     = std::string(s.Fields[paratype_id]);
                operation    = s.Fields[operation_id];
                name         = (std::string)s.Fields[name_id];
                defaultValue = (std::string)s.Fields[default_id];
                position     = s.Fields[position_id];
                isconst      = s.Fields[const_id];
                kind         = (std::string)s.Fields[kind_id];
                classifier   = (std::string)s.Fields[classifier_id];
                notes        = (std::string)s.Fields[notes_id];

                if (helper::tolower(kind) != "return") {
                    std::string guid = id_map[operation];

                    std::shared_ptr<MElement>  parent = MElement::Instances.find(guid)->second;

                    auto param = MParameter::construct(id, parent);

                    param->parent         = guid;
                    param->ClassifierName = paratype;
                    param->defaultValue   = defaultValue;
                    param->isReadOnly     = isconst;
                    param->name           = name;
                    param->mPosition      = position;
                    param->Direction      = kind;
                    if (!classifier.empty()) {
                        long cl = std::atol(classifier.c_str());

                        if (cl != 0) {
                            param->Classifier  = object_id_map[cl];
                        }
                    }
                    param->comment        = notes;

                    if (xrefClientMap.find(id) != xrefClientMap.end()) {
                        eaXref* prop = xrefClientMap.find(id)->second;

                        std::string lower = prop->get("lower");
                        std::string upper = prop->get("upper");

                        if (!lower.empty()) {
                            param->Multiplicity = lower;
                        } else {
                            param->Multiplicity = "1";
                        }
                        if (!upper.empty()) {
                            if ((!param->Multiplicity.empty())  && (upper != lower)) {
                                param->Multiplicity += "..";
                            }
                            param->Multiplicity     += upper;
                        }
                    }

                    auto o = std::dynamic_pointer_cast<MOperation>(parent);

                    o->Parameter.emplace_back(param);
                }
            }
        }
    } else {

    }
}
//
//  Load connectors
static void ea_eap_load_connectors(std::shared_ptr<MModel> aModel) {
    tSQLRecordSet* table = tables["connectors"];

    if (table != nullptr) {
        int connector_type_id = find_column_id("connector_type", table);
        int guid_id           = find_column_id("ea_guid", table);
        int connector_id      = find_column_id("connector_id", table);

        if ((connector_type_id != -1) && (guid_id != -1) && (connector_id != -1)) {
            std::string  type;
            std::string  id;

            for (auto & s : table->Records) {
                type       = helper::tolower(s.Fields[connector_type_id]);
                id         = (std::string)s.Fields[guid_id];
                //
                //  Different processing for different types.
                if (type == "aggregation") {
                    auto assoc = MAssociation::construct(id, nullptr);

                    fillassoc(s, assoc);
                } else if ((type == "generalization") || (type == "realisation")) {
                    auto  g    = MGeneralization::construct(id, nullptr, nullptr);

                    fillgeneralization(s, g);
                }
                else if ((type == "dependency") || (type == "abstraction") || (type == "usage")) {
                    std::shared_ptr<MStereotype> st;

                    if (type == "usage") {
                        st = aModel->StereotypeByName("use");
                    }
                    auto d = MDependency::construct(id, st, nullptr);

                    filldependency(s, d);
                }
                else if (type == "notelink") {
                    auto nl = MNoteLink::construct(id, nullptr, nullptr);

                    fillnotelink(s, nl);
                } else if (type == "templatebinding") {
                    auto  g    = MGeneralization::construct(id, nullptr, nullptr);

                    fillgeneralization(s, g);
                                            //
                    //  connect the parameters.
                    auto range = xrefClientMap.equal_range(id);
                    //
                    //  as we do not have control over the ordering of the various information
                    //  needed to setup the template binding we need to make it in two steps.
                    //
                    //  First we collect the parameters.
                    for (auto it = range.first; it != range.second; ++it) {
                        if ((it->second->mDescription.mName != "ClassifierRef") && (it->second->mDescription.mStruct.size() > 0)) {
                            auto fields = it->second->mDescription.mStruct.begin();
                            std::string guid;
                            std::string name;
                            std::string t;
                            std::string pos;
                            std::string etype;

                            if (fields->mName == "@ELEMENT") {
                                //
                                //  Setup the object parameter.
                                for (; fields != it->second->mDescription.mStruct.end(); ++fields) {
                                    if (fields->mName == "GUID") {
                                        guid = fields->mValue;
                                    } else if (fields->mName == "ActualName") {
                                        name = fields->mValue;
                                    } else if (fields->mName == "Type") {
                                        t = fields->mValue;
                                    } else if (fields->mName == "Pos") {
                                        pos = fields->mValue;
                                    } else {
                                    }
                                }
                                auto bindingarameter = new MParameter(guid, g);

                                bindingarameter->name = name;
                                bindingarameter->mPosition = atoi(pos.c_str());

                                g->mTemplateParameter[atoi(pos.c_str())] = bindingarameter->sharedthis<MParameter>();
                            }

                        }
                    }
                    //
                    //  In the second run we fillin the classifier reference if available.
                    for (auto it = range.first; it != range.second; ++it) {
                        if ((it->second->mDescription.mName == "ClassifierRef") && (it->second->mBehavior == "actual")) {
                            std::string supplier = it->second->mSupplier;

                            auto param = MParameter::Instances.find(supplier);
                            if (param != MParameter::Instances.end()) {
                                std::dynamic_pointer_cast<MParameter>(param->second)->Classifier = it->second->mDescription.mValue;
                            }
                        }
                    }
                }
            }
            tSQLRecordSet* props = tables["connectortags"];

            if (props != nullptr) {
                int objid = find_column_id("elementid", props);
                int propid = find_column_id("property", props);
                int valueid = find_column_id("value", props);

                if ((objid != -1) && (propid != -1) && (valueid != -1)) {
                    long oid;
                    std::string name;
                    std::string value;
                    auto & id_map = id_maps["connectors"];

                    for (auto& p : props->Records) {
                        oid = p.Fields[objid];
                        name = (std::string)p.Fields[propid];
                        value = (std::string)p.Fields[valueid];

                        id = id_map[oid];
                        auto e = MElement::Instances.find(id);

                        if ((e != MElement::Instances.end()) && (e->second != nullptr)) {
                            if (!e->second->AddTag(name, value)) {
                                std::cerr << "Cannot add tagged value " << name << " to object " << e->second->name << ". Probably double defined.\n";
                            }
                        }

                    }
                }
            }

        }
    } else {

    }
}
//
//  Load the tagged values.
static void ea_eap_load_tagged_values(std::shared_ptr<MModel> aModel) {
    (void)aModel;
}
//
//  Load the xref table
static void ea_eap_load_xref(std::shared_ptr<MModel> aModel) {
    tSQLRecordSet* table = tables["xref"];

    if (table != nullptr) {
        int ref_id         = find_column_id("xrefid", table);
        int client_id      = find_column_id("client", table);
        int supplier_id    = find_column_id("supplier", table);
        //
        int name_id        = find_column_id("name", table);
        int type_id        = find_column_id("type", table);
        int vis_id         = find_column_id("visibility", table);
        int namespace_id   = find_column_id("namespace", table);
        int req_id         = find_column_id("requirement", table);
        int constraint_id  = find_column_id("constraint", table);
        int behavior_id    = find_column_id("behavior", table);
        int partition_id   = find_column_id("partition", table);
        int description_id = find_column_id("description", table);
        int link_id        = find_column_id("link", table);

        if ((ref_id != -1) && (client_id != -1) && (supplier_id != -1)) {
            std::string ref;
            std::string client;
            std::string supplier;


            for (auto & s : table->Records) {
                ref      = (std::string)(s.Fields[ref_id]);
                client   = (std::string)s.Fields[client_id];
                supplier = (std::string)s.Fields[supplier_id];

                eaXref* x   = eaXref::construct(s.Fields[type_id]);

                x->mXrefId = ref;
                if ((!client.empty()) && (client != "<none>")) {
                    x->mClient = client;
                    xrefClientMap.insert(std::pair<std::string, eaXref*>(client, x));
                }
                if ((!supplier.empty()) && (supplier != "<none>")) {
                    x->mSupplier = supplier;
                    xrefSupplierMap.insert(std::pair<std::string, eaXref*>(supplier, x));
                }
                //
                //
                x->mName  = (std::string)s.Fields[name_id];
                x->mVisibility = getvisibility(s.Fields[vis_id]);
                x->mNamespace = (std::string)(s.Fields[namespace_id]);
                x->mRequirement = (std::string)(s.Fields[req_id]);
                x->mConstraint = (std::string)(s.Fields[constraint_id]);
                x->mBehavior = (std::string)(s.Fields[behavior_id]);
                x->mPartition = (std::string)(s.Fields[partition_id]);
                x->setupDescription(s.Fields[description_id]);
                x->mLink = (std::string)(s.Fields[link_id]);

                xrefIdMap.insert(std::pair<std::string, eaXref*>(ref, x));
            }
        }
    }
}
/*
 * This is the only function that gets used by main program
 */
std::shared_ptr<MModel> ea_eap_modelparser(const char* filename, const char* directory)
{
    std::string        path;
    //
    //  Set the directory to the current working directory.
	path = helper::getcwd();
	//
    //  First check for relative directory and create it if needed.
    if (directory[0]!='/') {
        path = path + "/" + directory;
    } else {
        path = directory;
    }
    CPath outputdir = path;

    outputdir.Create();

    model = MModel::construct();
    
    if (model) {
        //
        //  First we need to load the basic tables that we need to load the model elements
        ea_eap_load_tables(filename);
        //
        //  Now its time to change the directory where to generate the model into.
        helper::chdir(path);
        //
        //  Load the xref tables
        ea_eap_load_xref(model);
        //
        //  Load the stereotypes
        ea_eap_load_stereotypes(model);
        //
        //  Load the tagged values.
        ea_eap_load_tagged_values(model);
        //
        //  Load the root model packages.
        ea_eap_load_roots(model);
        //
        //  Load packages
        ea_eap_load_objects(model);
        //
        //  Load operations
        ea_eap_load_operations(model);
        //
        //  Load attributes
        ea_eap_load_attributes(model);
        //
        //  Load OperationParameters
        ea_eap_load_opparams(model);
        //
        //  Here we are doing some fixups because the DB is not a
        //  tree where we could check against parents on the fly.
        //
        //  Connect parents.
        for (auto & e : MElement::Instances) {
            if (e.second->parent) {
                e.second->parent->Add(e.second);
            } else {
                //
                //  All elements that have no parent associated with
                //  are tryed to put into the model object.
                model->Add(e.second);
            }
        }
        //
        //  For normal classes we need to check if we have to re-construct
        //  them because they are part of an external package.
        std::map<std::shared_ptr<MElement>, std::string> re;

        for (auto & cb : MElement::Instances) {
           //  Only process plain classes.
            if (cb.second->type == eElementType::Class) {
                //
                //  Check along the parents if this should be an external class.
                auto parent = cb.second->parent;

                while (parent) {
                    if ((parent->type == eElementType::Class) || (parent->type == eElementType::Package)) {
                        parent = parent->parent;
                    } else {
                        switch (parent->type) {
                        case eElementType::ExternPackage:
                            re.insert(std::pair<std::shared_ptr<MElement>, std::string>(cb.second, "Extern"));
                            break;
                        default:
                            break;
                        }
                        break;
                    }
                }
            }
        }
        //
        //  Now reconstruct the classes found.
        for (auto r : re) {
             //
            //  Remove the class from the various internal maps.
            MClass::Instances.erase(r.first->id);
            MElement::Instances.erase(r.first->id);
            //
            // Drop the class from the parent.
            r.first->parent->Delete(r.first);
            //
            // Now create the appropriate class variant and attach it to the parent.
            auto reconstructed = CClassBase::construct(r.first->id, r.second, r.first->parent);

            *reconstructed = *std::dynamic_pointer_cast<MClass>(r.first);
            //
            //  SetName called to do some weird background processing.
            reconstructed->name = std::string(r.first->name);
            //
            //  reset the type.
            reconstructed->type = eElementType::ExternClass;
            //
            // Add it to the parent.
            r.first->parent->Add(reconstructed);
        }
        //
        //  Load connectors
        ea_eap_load_connectors(model);
        //
        //  Load diagrams.
        ea_eap_load_diagrams(model);
        //
        //
        //  Fill the classifiers in composite types.
        for (auto& g : gTypesToComplete) {
            if (!g->name.empty()) {
                // find the namespace from the packages.
                g->mNameSpace = findnamespace(g);
                //
                //  Setup the type tree and collect all that have an incomplete setup.
                g->mTypeTree = TypeNode::parse(g->name);
                if (!g->mTypeTree.isCompositeType()) {
                    g->mTypeTree.mClassifier = g;
                }
                g->mTypeTree.fill(g->mNameSpace);
                g->add();
            } else {
                std::cerr << "No name for element-id: " << g->id << std::endl;
            }
        }
    }

    id_maps.clear();

    for (auto & t : tables) {
        delete t.second;
    }
    tables.clear();
    for (auto & i : xrefIdMap) {
        delete i.second;
    }
    xrefIdMap.clear();
    xrefClientMap.clear();
    xrefSupplierMap.clear();

    return (model);
}

