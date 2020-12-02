class Clock:
    def __getitem__(self, id):
        now = datetime.datetime.now()
        ids = {
            "time": time.time,
            "year": now.year,
            "month": now.month,
            "day": now.day,
            "hour": now.hour,
            "minute": now.minute,
            "second": now.second,
            # No time_utc?
            "year_utc": now.utcnow().year,
            "month_utc": now.utcnow().month,
            "day_utc": now.utcnow().day,
            "hour_utc": now.utcnow().hour,
            "minute_utc": now.utcnow().minute,
            "second_utc": now.utcnow().second,
        }
        return ids[id]
