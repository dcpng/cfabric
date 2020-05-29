#ifndef PLUGIN_SYS_H
#define PLUGIN_SYS_H

#define CFABRIC_MOD_NAME "cfabric"

typedef int TClassHandle;
typedef struct INamespaceContext_struct INamespaceContext;
//struct IInterface
//{
//    /*
//    Cast the current object to a different Type
//    */
//    void *(*Cast)(void * /*this*/, TClassHandle * /*IClass*/);
//};

/*This is the interface that all concrete classes must extend. It MUST be declared as the 
FIRST field of the class*/
struct IObject
{
    void (*_objDstr)(void ** /*this*/);
    /*constructor for creating an instance or return a singleton instance of the class
    arg:
        aCtx -> used by the Constructor perform any initialisation/stat-updates on the
            on the plugin context
        aClassDef -> is the reference to the class definition field that was registered
            by calling INamespaceContext.MapClassByName(...).
    returns:
        A new object or singleton depends on the implementation of the constructor
    */
    void *(*_objCstr)(INamespaceContext **aCtx);
};
#define IObjectHandler -1

/*plugin loader is a collection to load*/
struct INamespaceContext_struct
{
    /*
    Map a class implementation to a canonical name
    args:
        aClass -> Address to the Implementation of the Concrete Class. 
        aName -> the canonical Class name to address the aClass object
    returns:
        0 -> OK
        -1 -> ERR: Canonical Class name is already used a previous call of the method
            or MapXxxByName methods
    Fatal:
        if aClass or aName is set to NULL or aName is malformed
    */
    int (*MapClassByName)(void *aThis, struct IObject *aClass, char *aName);

    /*
    args:
        aName -> the canonical Class name to address the aClass object
    return:
        IClass Object that will use for creating instances of the class. This method
        will ALWAYS return a value.
    Fatal:
        if aName is set to NULL or malformed
    */
    TClassHandle (*GetClassHandleByName)(void *aThis, char *aName);

    /*
    Create a new instance or return a singleton instance of aClass depending on the
    implementation of the class
    args:
        aClass -> The IClass object returned by the GetClassHandleByName method
    return:
        a) address of the instance of the class if the aClass refers to a Concrete class
        b) NULL if the aClass is either an interface or undefined
    */
    void *(*GetInstanceByClassHandle)(void *aThis, TClassHandle aClass);
};

typedef INamespaceContext **(*TOpenNamespaceFn)(char *);

/**
 * This function interface much be implemented by other plugin modules
*/
typedef void (*TModInstaller)(TOpenNamespaceFn nsFn);
#endif
