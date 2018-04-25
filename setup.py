from setuptools import setup, Extension, find_packages

p4t_native = Extension(
    'p4t_native',
    ['p4t_native/p4t/{:s}'.format(src) for src in [
        'common.cpp',

        'opt/chain_algos.cpp',
        'opt/oi_algos.cpp',
        'opt/boolean_minimization.cpp',
        'opt/expansion_algos.cpp',
        'opt/distribution_algos.cpp',
        'opt/intersections_opt.cpp',
        'opt/updates.cpp',

        'utils/python_utils.cpp',

        'p4t_native.cpp',
        'p4t_native_ext.cpp',
    ]],
    libraries=['boost_python3', 'boost_numpy3', 'gomp'],
    include_dirs=['p4t_native'],
    extra_compile_args=['-fopenmp', '-std=c++17', '-Wall', '-msse4.1', '-O3']
)

setup(
    name='cls',
    version='0.0.1',
    description='Classification framework and algorithms',
    license='Apache-2.0',
    packages=find_packages(exclude=['test*']),
    ext_modules=[p4t_native],
    install_requires=['click', 'numpy', 'scipy'],
    setup_requires=['pytest-runner'],
    tests_require=['pytest']
)
