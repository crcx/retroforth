class RNG:
    def __call__(self, seed=None, new_seed=True):
        return random.randint(-2147483647, 2147483646)

