#ifndef CTOSCALLBACK_H
#define CTOSCALLBACK_H

class CtoSCall {
public:

};

class CtoSCallback
{
public:
    CtoSCallback();
    virtual ~CtoSCallback() {};
    virtual void onResponse() = 0;
    virtual void onFailure() = 0;

private:
    CtoSCall *m_call;
};

#endif // CTOSCALLBACK_H
