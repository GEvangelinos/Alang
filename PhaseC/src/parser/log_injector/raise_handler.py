import inspect
import os
from typing import Type


def raise_error(error_type: Type[BaseException], error_message: str) -> None:
        erroneous_frame = inspect.stack()[1]
        filename = os.path.basename(erroneous_frame.filename)
        line_number = erroneous_frame.lineno
        function_name = erroneous_frame.function
        full_error_message = f"{filename}:{line_number} -> {function_name}():{error_message}"
        raise error_type(full_error_message)
