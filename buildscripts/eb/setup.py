from setuptools import setup

setup(name='eb',
      version='1.0',
      packages=[
          'eb',
      ],
      install_requires=[
          'nose==1.3.7',
      ],
      setup_requires=[
          'nose==1.3.7'
      ],
      entry_points={
          'console_scripts': [
              'eb = eb.cli:main',
          ]
      },
      )