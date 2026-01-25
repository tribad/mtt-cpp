#pragma once
#ifndef ACLASS_INC
#define ACLASS_INC
//
//  type aliases
///
/// Demonstration of class member attributes and operations.
///
class AClass {
public:
    ///
    /// @brief Do something
    ///
    /// @param[] param1 An integer
    /// @return True or False?
    ///
    bool APublicMethod(const int param1) noexcept;
public:
    int           APublicInt;      // An Integer
protected:
    float         AProtectedFloat;
private:
    bool          APrivateBool;
    static double AStaticDouble;
};
//
//  These are the operations defined with package scope.
void APackageOperation() ;

#endif  // ACLASS_INC
