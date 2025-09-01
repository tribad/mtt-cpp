//
// Copyright 2020 Hans-Juergen Lange <hjl@simulated-universe.de>
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

#include "cgeneralization.h"
#include "mparameter.h"
#include "cparameter.h"
#include "moperation.h"
#include "coperation.h"
#include "chtmlpageclass.h"

CHtmlPageClass::CHtmlPageClass(const std::string&aId, std::shared_ptr<MElement> e) : CCxxClass(aId, e) {
    type = eElementType::HtmlPageClass;
}

void CHtmlPageClass::Prepare() {
    auto b = CClassBase::mByFQN.find("CHtmlPage");

    if (b != CClassBase::mByFQN.end()) {
        //
        //  Setup generalization
        auto newgen = new CGeneralization(id + "gen", sharedthis<MElement>());
        auto gen = newgen->sharedthis<CGeneralization>();

        gen->base = std::dynamic_pointer_cast<MElement>(b->second);
        gen->derived = sharedthis<MElement>();
        gen->visibility = vPublic;
        //
        //  Create the links.
        Base.emplace_back(std::dynamic_pointer_cast<CClassBase>(*gen->base), gen);
        std::dynamic_pointer_cast<CClassBase>(*gen->base)->Derived.emplace_back(sharedthis<CClassBase>(), gen);

        Generalization.emplace_back(gen);
        //
        //  Create the default constructor
        auto newctor = new COperation(id + "-ctor", sharedthis<MElement>());
        Operation.emplace_back(id + "-ctor");
        newctor->visibility = vPublic;
        newctor->name = name;
        newctor->Specification = ": CHtmlPage(\"" + name + "\")";
        //
        //  Create the virtual destructor
        auto newdtor = new COperation(id + "-dtor", sharedthis<MElement>());
        Operation.emplace_back(id + "-dtor");
        newdtor->visibility = vPublic;
        newdtor->name = "~" + name;
        newdtor->isAbstract = true;
        //
        //  Handle Request
        auto handlereq = new COperation(id + "-request-handler", sharedthis<MElement>());
        Operation.emplace_back(id + "-request-handler");
        handlereq->visibility = vPublic;
        handlereq->name = "HandleRequest";
        handlereq->isAbstract = true;
        //
        //  Create Parameter
        auto requestpara = new CParameter(id + "-request-handler-req-parameter", MElementRef(id + "-request-handler"));
        requestpara->visibility = vPublic;
        requestpara->name       = "a_req";
        requestpara->ClassifierName = "tHttpRequest*";

        handlereq->Parameter.emplace_back(MElementRef(id + "-request-handler-req-parameter"));
        //
        //  Create return type.
        auto requestret = new CParameter(id + "-request-handler-req-return", MElementRef(id + "-request-handler"));
        requestret->visibility = vPublic;
        requestret->name       = "response";
        requestret->ClassifierName = "tHttpResponse*";
        requestret->Direction = "return";
        requestret->defaultValue = "nullptr";

        handlereq->Parameter.emplace_back(MElementRef(id + "-request-handler-req-return"));

        //
        //  Process Messages.
        auto process = new COperation(id + "-process", sharedthis<MElement>());
        Operation.emplace_back(id + "-process");
        process->visibility = vPublic;
        process->name = "Process";
        process->isAbstract = true;

        //
        //  Create Parameter
        requestpara = new CParameter(id + "-process-req-parameter", MElementRef(id + "-process"));
        requestpara->visibility = vPublic;
        requestpara->name       = "a_req";
        requestpara->ClassifierName = "tHttpRequest*";

        process->Parameter.emplace_back(MElementRef(id + "-process-req-parameter"));

        requestpara = new CParameter(id + "-process-msg-parameter", MElementRef(id + "-process"));
        requestpara->visibility = vPublic;
        requestpara->name       = "a_msg";
        requestpara->ClassifierName = "std::shared_ptr<tMsg>";

        process->Parameter.emplace_back(MElementRef(id + "-process-msg-parameter"));
        //
        //  Create return type.
        requestret = new CParameter(id + "-process-req-return", MElementRef(id + "-process"));
        requestret->visibility = vPublic;
        requestret->name       = "response";
        requestret->ClassifierName = "tHttpResponse*";
        requestret->Direction = "return";
        requestret->defaultValue = "nullptr";

        process->Parameter.emplace_back(MElementRef(id + "-process-req-return"));

    }
}


void CHtmlPageClass::SetFromTags(const std::string& name, const std::string&value)
{
}

void CHtmlPageClass::Dump(std::shared_ptr<MModel> aModel) {
    auto b = CClassBase::mByFQN.find("CHtmlPage");

    if (b != CClassBase::mByFQN.end()) {
        //
        //  Add header
        extramodelheader.emplace_back(std::dynamic_pointer_cast<CClassBase>(b->second));
        //
    }
    CCxxClass::Dump(aModel);
}
