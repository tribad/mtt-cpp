# Users Manual

This manual is intended for those people who only want to use the generator.
It explains the things that are related to code-generation from UML models but is no UML modeling handbook.

You will find some areas where I describe things that are not directly related to the generator. There I only can talk
about my own experiences and believe in certain things. So don't be offended by that. Please.

Maybe others can fetch some ideas out of it, to implement other approaches.

## Philosophy of the generator

It's always helpful to understand the philosophy of a software first before using it. Cause sometime you have a 
complete different idea how things should work.

### Why creating only skeletons?
It may sound a good idea to create all code from the model. This would possible with sequences and activities. I thought
of it many times and even started with some implementation that still lingers in the code base.

But in the end the effort for the creation of the necessary diagrams, that must be detailed enough to accomplish the goal, 
is far to high.

Another problem arises in an professional development company. There you would need to either qualify the tool, so proof 
that the generated code is always correct, even if the generator may crash inbetween, that it is possible to identify that and
react to it.

For this qualification process you would need to have all code covered by requirements, and have tests in place to proof that
these requirements are met. Everything around all that effort needs to be done after any change to the software. So a really 
complete automated environment.

Or you do a review of the generated code afterwards. 

So it makes far more sense to create only the skeletons and do the implementation manually and review it afterwards.

### Why is it a standalone solution?

Make it an integral part of any UML modeler, there are not many out there that I would use in some sort of professional 
environment, would require the support of different language bindings. Enterprise Architect has a .NET library to access
some functions. IBM Rhapsody has a Java Library. And other may use some different language.

As all these modelers conform in one way or the other on the UML meta model, it is far more easy to create these input plugins,
that deal with the model storage.

As a standalone application it can even been run in a CI/CD pipeline. So it's more a thing of flexibility.

### How does the generator identify the parts to generate?

Mainly it is done through stereotypes that can be attached to the model elements. If there are dependencies to not stereotyped
model elements it is determined from the context, like for Interface Classes. They may be used for C++ and Java and if I am 
right are stored differently in Enterprise Architect and StarUML. One attaching a stereotype to a class the other has an 
interface type in its internal model.

The java generator is not part of the code base. But I know that a friend of mine created Java generator classes and it worked.

## The manual
Here we start with the manual. All examples are based on a StarUML model. As UML :registered: is somewhat a standard you will find similar
behaviour for the Enterprise Architect as well.

### General approach
To generate files from a UML model you deal with two different ways to make things visible to the generator.

* Stereotypes
* Tagged Values

#### Stereotypes
These are as abstract as an UML model is. They help distinguish between variations of Model Elements. For example a Class.

In short: Classes are containers that can have attributes and/or methods. So if you have something that has attributes and/or methods,
it can be represented as class. But without further information this thing can not be distinguished between a class that 
should be coded, or a class that represents only some sort of meta model element, or is a task, or something completely different
that by accident has attributes and/or methods.

These are the place where stereotypes are used. They help distinguish, not only in the generator, between variations of model elements.

#### Tagged Values
Tagged values are simple name value pair that can be attached to a model element. They are often used to connect the abstract
world of the model to the real world.

These are extensively used with generator, because that's his purpose, transfer the abstract model into the real world.



