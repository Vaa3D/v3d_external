#ifndef SLOTMERGER_H
#define SLOTMERGER_H

class SlotMerger;

// SlotStatus tracks whether a particular Qt slot is currently running.
class SlotStatus
{
public:
    friend class SlotMerger;
    SlotStatus();

    bool running() const {return isRunning;}

private:
    bool isRunning;
    int skippedCallCount;
};

// Slot merger helps merge multiple calls of a Qt slot into one call.
// Optionally slow down those too-fast update() slot calls
// like this:
//     void MyNaLockableData::update() {
//         SlotMerger updateMerger(statusOfUpdateSlot);
//         if (! updateMerger.shouldRun()) return;
//         ...
class SlotMerger
{
public:
    explicit SlotMerger(SlotStatus& statusParam);
    virtual ~SlotMerger();
    bool shouldRun() const;
    int skippedCallCount() const;

private:
    SlotStatus& status;
    bool bShouldRun;
};

#endif // SLOTMERGER_H
