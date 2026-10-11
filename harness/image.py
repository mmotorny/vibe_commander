"""Just enough image handling to turn a QEMU screen dump into a PNG."""
import struct
import zlib
from dataclasses import dataclass


@dataclass(frozen=True)
class Image:
    width: int
    height: int
    rgb: bytes                    # height rows of width RGB triples

    @classmethod
    def from_ppm(cls, data):
        """Parse a binary PPM (P6) file, the format QEMU dumps screens in."""
        fields = []
        pos = 0
        while len(fields) < 4:
            end = pos
            while not data[end:end + 1].isspace():
                end += 1
            token = data[pos:end]
            pos = end + 1
            if token.startswith(b"#"):          # a comment runs to end of line
                pos = data.index(b"\n", end) + 1
            elif token:
                fields.append(token)
        magic, width, height, maxval = fields
        if magic != b"P6" or maxval != b"255":
            raise ValueError("not an 8-bit binary PPM")
        width, height = int(width), int(height)
        return cls(width, height, data[pos:pos + width * height * 3])

    def scaled(self, x, y):
        """Enlarge by whole factors, repeating pixels so edges stay sharp."""
        stride = self.width * 3
        rows = []
        for top in range(0, len(self.rgb), stride):
            row = self.rgb[top:top + stride]
            wide = b"".join(row[i:i + 3] * x for i in range(0, stride, 3))
            rows.extend([wide] * y)
        return Image(self.width * x, self.height * y, b"".join(rows))

    def to_png(self):
        stride = self.width * 3
        raw = bytearray()
        previous = None
        for top in range(0, len(self.rgb), stride):
            row = self.rgb[top:top + stride]
            if row == previous:                 # filter 2: same as row above
                raw += b"\x02" + bytes(stride)
            else:                               # filter 0: unfiltered
                raw += b"\x00" + row
            previous = row

        def chunk(kind, body):
            return (struct.pack(">I", len(body)) + kind + body
                    + struct.pack(">I", zlib.crc32(kind + body)))

        header = struct.pack(">IIBBBBB", self.width, self.height, 8, 2, 0, 0, 0)
        return (b"\x89PNG\r\n\x1a\n" + chunk(b"IHDR", header)
                + chunk(b"IDAT", zlib.compress(bytes(raw), 9))
                + chunk(b"IEND", b""))
