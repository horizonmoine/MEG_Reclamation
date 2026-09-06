import sys
print("Python executable:", sys.executable)
try:
    import numpy as np
    print("NumPy available:", np.__version__)
except Exception as e:
    print("NumPy error:", e)

try:
    import PIL
    print("PIL available:", PIL.__version__)
except Exception as e:
    print("PIL error:", e)
