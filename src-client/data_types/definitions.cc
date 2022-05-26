#include "definitions.h"
#include "bytes.h"

Direction get_direction(Bytes &bytes) {
    char c = bytes.get_next_byte();
    if (c >= 4) {
        throw BytesDeserializationException();
    }
    return (Direction) c;
}
