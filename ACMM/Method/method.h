#ifndef METHOD_H
#define METHOD_H

//做个观察者模式 serial和decode完成报文解析后，各个观察者自行决定怎样去执行自己的part
class Method{
public:
    explicit Method();
    virtual ~Method();
    virtual void getInfo() = 0;
    virtual void pretreatment() = 0;
//    virtual void assigAbstractObject(Method * arg) = 0;
};

#endif // METHOD_H
