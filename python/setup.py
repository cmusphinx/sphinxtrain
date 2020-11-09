from setuptools import setup

setup(name='cmusphinx',
      version='1.0.1',
      description='CMU Sphinx Python modules for speech processing',
      author='David Huggins-Daines',
      author_email='dhdaines@gmail.com',
      url='http://www.cmusphinx.org/',
      packages=['cmusphinx', 'cmusphinx.feat'],
      requires=['numpy', 'scipy'],
      classifiers=[
          "Development Status :: 2 - Pre-Alpha",
          "Programming Language :: Python :: 3",
          "Programming Language :: Python :: 3.7",
          "License :: OSI Approved :: MIT License",
          "Operating System :: OS Independent",
      ],
)
