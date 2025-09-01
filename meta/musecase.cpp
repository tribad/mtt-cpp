//
// Copyright 2015 Hans-Juergen Lange <hjl@simulated-universe.de>
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
#include "musecase.h"
#include "cusecase.h"

std::map<std::string, std::shared_ptr<MUseCase>> MUseCase::Instances;

MUseCase::MUseCase(const std::string&aId, std::shared_ptr<MElement> aParent) : MElement(aId, aParent) {
    type = eElementType::UseCase;
    MUseCase::Instances.insert(std::pair<std::string, std::shared_ptr<MUseCase>>(id, sharedthis<MUseCase>()));
}


std::shared_ptr<MUseCase> MUseCase::construct(const std::string&aId, std::shared_ptr<MElement> aParent)
{
    auto retval = new CUseCase(aId, aParent);

    return retval->sharedthis<MUseCase>();
}
