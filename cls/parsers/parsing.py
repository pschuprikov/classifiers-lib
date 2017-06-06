from itertools import repeat, chain
from functools import wraps

from cls.classifiers.simple import BasicClassifier
from cls.vmrs.simple import SimpleVMR, SimpleVMREntry


class Filter:
    def __init__(self, value, mask): 
        self.value = list(value)
        self.mask = list(mask)

    def __add__(self, other):
        return Filter(self.value + other.value, self.mask + other.mask)

    def __str__(self):
        return "".join(
            str(bit) if mask_bit else '*'
            for (bit, mask_bit) in zip(self.value, self.mask)
        )

    def __len__(self):
        return len(self.value)

    def parse_line(line):
        raise NotImplementedError



def classifier_format(*widths):
    def decorator(func):
        @wraps(func)
        def wrapper(line):
            return func(line)
        wrapper.width = sum(widths)
        return wrapper
    return decorator


def _parse_range(range):
    return tuple(map(int, range.split(' : ')))


def _int_to_bit(x, num_bits):
    result = [int(digit) for digit in bin(x)[2:]]
    result.extend(repeat(0, num_bits - len(result)))
    return result


def _octets_to_bits(s):
    return list(chain.from_iterable(
        _int_to_bit(int(octet), 8) for octet in s.split('.')
        ))


def _ip_to_filter(ip):
    if '/' in ip:
        ip, nm = ip.split('/')
    else:
        ip, nm = ip, '32'

    value = _octets_to_bits(ip)

    if '.' in nm:
        mask = list(1 - x for x in _octets_to_bits(nm))
    else:
        mask = list(chain(repeat(1, int(nm)), repeat(0, 32 - int(nm))))

    return Filter(value, mask)


def _maybe_exact_to_filter(x, num_bits):
    if int(x) < 0:
        return Filter(repeat(1, num_bits), repeat(0, num_bits))
    else:
        return Filter(_int_to_bit(int(x), num_bits), repeat(1, num_bits))


def _field_to_filter(proto, num_bits):
    value, mask = (_int_to_bit(int(x, 16), num_bits) for x in proto.split('/'))
    return Filter(value, mask)


def _chars_to_filter(str):
    return Filter(
        (1 if c == '1' else 0 for c in str),
        (1 if c != '*' else 0 for c in str)
    )


def _pylist_to_filters(lst):
    return [_chars_to_filter(s) for s in eval(lst)]


def text(bit_width):
    @classifier_format(bit_width)
    def parser(line):
        return [_chars_to_filter(line)]
    return parser


@classifier_format(32, 32, 4, 4)
def icnp(line):
    src_ip, dst_ip, _, _, x1, x2 = line.split('\t')
    return [
         _ip_to_filter(src_ip) + _ip_to_filter(dst_ip) +
         _maybe_exact_to_filter(x1, 4) + _maybe_exact_to_filter(x2, 4)
         ]


@classifier_format(32, 32, 8, 16)
def classbench(line):
    src_ip, dst_ip, in_port, out_port, proto, eth_type, _ = line[1:].split('\t')
    return [
        _ip_to_filter(src_ip) + _ip_to_filter(dst_ip) +
        _field_to_filter(proto, 8) + _field_to_filter(eth_type, 16)
    ]


@classifier_format(32, 32, 16, 16, 8)
def classbench_expanded(line):
    src_ip, dst_ip, proto, in_port, out_port = line[1:].split('\t')
    return [
        _ip_to_filter(src_ip) + _ip_to_filter(dst_ip) + x + y + _field_to_filter(proto, 8)
        for x in _pylist_to_filters(in_port)
        for y in _pylist_to_filters(out_port)
    ]


def read_classifier(clsf_format, lines):
    vmr = SimpleVMR(clsf_format.width)
    for i, line in enumerate(lines):
        for flt in clsf_format(line):
            vmr.append(SimpleVMREntry(flt.value, flt.mask, i + 1, 0))
    return BasicClassifier(vmr)
