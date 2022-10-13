# Install a copy of Python modules for use in SphinxTrain scripts
# (you can also install them separately as a real package)
install(DIRECTORY cmusphinx
  DESTINATION ${CMAKE_INSTALL_DATADIR}/sphinxtrain/python
  PATTERN __pycache__ EXCLUDE
  PATTERN "*.pyc" EXCLUDE
  PATTERN "*.pyo" EXCLUDE)
