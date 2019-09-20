class EventTimer
{
  private:
    bool isRunning = false;
    uint32_t duration;
    uint32_t startTime;
  public:
    void Start(uint32_t dur);
    void Cancel();
    bool CheckExpired();
};

void EventTimer::Start(uint32_t dur)
{
  isRunning = true;
  startTime = millis();
  duration = dur;
}

void EventTimer::Cancel()
{
  isRunning = false;
}

bool EventTimer::CheckExpired()
{
  uint32_t currTime = millis();
  bool retVal = false;
  if ((currTime - startTime >= duration) && isRunning)
  {
    isRunning = false;
    retVal = true;
  }

  return retVal;
}
