"""Время жизни ресурса C API. / Lifetime of an owned C API resource."""

import weakref

from .languages import message_text


class OwnedHandle:
    """Дескриптор с однократным освобождением. / A handle released exactly once."""

    def _own(self, pointer, release, closed_message):
        self._pointer = pointer
        self._closed_message = closed_message
        self._finalizer = weakref.finalize(self, release, pointer)

    @property
    def closed(self) -> bool:
        return not self._finalizer.alive

    @property
    def handle(self) -> int:
        if self.closed:
            raise RuntimeError("Берёста / Beresta: " + message_text(self._closed_message))
        return self._pointer

    def close(self) -> None:
        self._finalizer()
