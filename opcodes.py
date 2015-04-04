mist32_opcodes = {
    0: 'add',
    1: 'sub',
    2: 'mull',
    3: 'mulh',
    4: 'udiv',
    5: 'umod',
    6: 'cmp',
    7: 'div',
    8: 'mod',
    9: 'neg',

    14: 'addc',
    16: 'inc',
    17: 'dec',
    28: 'sext8',
    29: 'sext16',

    64: 'shl',
    65: 'shr',
    69: 'sar',
    72: 'rol',
    73: 'ror',

    96: 'and',
    97: 'or',
    98: 'xor',
    99: 'not',
    100: 'nand',
    101: 'nor',
    102: 'xnor',
    103: 'test',

    106: 'wl16',
    107: 'wh16',
    108: 'clrb',
    109: 'setb',
    110: 'clr',
    111: 'set',
    112: 'revb',
    113: 'rev8',
    114: 'getb',
    115: 'get8',

    118: 'lil',
    119: 'lih',
    122: 'ulil',

    128: 'ld8',
    129: 'ld16',
    130: 'ld32',
    131: 'st8',
    132: 'st16',
    133: 'st32',

    136: 'push',
    137: 'pushpc',
    144: 'pop',

    154: 'ld8',  # ldd8
    155: 'ld16', # ldd16
    156: 'ld32', # ldd32
    157: 'st8',  # std8
    158: 'st16', # std16
    159: 'st32', # std32

    160: 'bur',
    161: 'br',
    162: 'b',
    163: 'ib',
    
    176: 'bur', # burn
    177: 'br',  # brn
    178: 'b',   # bn

    192: 'srspr',
    193: 'srpdtr',
    194: 'srpidr',
    195: 'srcidr',
    196: 'srmoder',
    197: 'srieir',
    201: 'srkpdtr',
    202: 'srmmur',
    203: 'sriosr',
    204: 'srtidr',
    205: 'srppsr',
    206: 'srppcr',
    207: 'sruspr',
    208: 'srppdtr',
    209: 'srptidr',
    211: 'srpsr',
    212: 'srfrcr',
    213: 'srfrclr',
    214: 'srfrchr',
    215: 'srpflagr',
    216: 'srfi0r',
    217: 'srfi1r',

    224: 'srspw',
    225: 'srpdtw',
    229: 'srieiw',
    233: 'srkpdtw',
    234: 'srmmuw',
    237: 'srppsw',
    238: 'srppcw',
    239: 'sruspw',
    240: 'srppdtw',
    241: 'srptidw',
    242: 'sridtw',
    243: 'srpsw',
    247: 'srpflagw',
    255: 'srspadd',

    256: 'nop',
    257: 'halt',
    258: 'move',
    259: 'movepc',

    288: 'swi',
    289: 'tas',
    290: 'idts',
}
