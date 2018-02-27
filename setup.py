from setuptools import setup, Extension, find_packages

p4t_native = Extension(
    'p4t_native',
    ['p4t_native/{:s}'.format(src) for src in [
        'common.cpp',
        'p4t_native.cpp',
        'p4t_native_ext.cpp',
        'chain_algos.cpp',
        'oi_algos.cpp',
        'expansion_algos.cpp',
        'boolean_minimization.cpp',
        'distribution_algos.cpp'
    ]],
    libraries=['boost_python', 'gomp'],
    include_dirs=['p4t_native'],
    extra_compile_args=['-fopenmp', '-std=c++14', '-Wall']
)

setup(
    name='cls',
    version='0.0.1',
    description='Classification framework and algorithms',
    license='Apache-2.0',
    packages=find_packages(exclude=['test*']),
    ext_modules=[p4t_native],
    install_requires=['bitstring', 'oct2py', 'click', 'numpy', 'scipy'],
    setup_requires=['pytest-runner'],
    tests_require=['pytest']
)
