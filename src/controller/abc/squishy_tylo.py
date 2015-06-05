def space_string(phrase):
    squishy_stuff = [{
            'current':  lambda x: x in ([str(y) for y in range(0,10)]+['.']),
            'previous': lambda x: x in ([str(y) for y in range(0,10)]+['.'])
        }, {
            'current':  lambda x: x == '=',
            'previous': lambda x: x in ['+', '-', '*', '/', '^', '%']
        }
    ]

    previous_c = ' '
    needle = ''
    for current_c in phrase.split(' '):
        if not any([(y['current'](current_c) and y['previous'](previous_c)) for y in squishy_stuff]):
            needle += ' '
        needle += current_c
        previous_c = current_c

    return needle
