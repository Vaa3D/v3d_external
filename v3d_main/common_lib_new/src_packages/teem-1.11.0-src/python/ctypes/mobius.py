
import sys
import teem
import pullDemo
import ctypes

npos = teem.nrrdNew()
vol = pullDemo.volLoad(['../../data/fmob-c4h.nrrd:scalar:V'], './', 'ds:1,5', 1)
infoList = ['h-c:V:val:0:-1',
            'hgvec:V:gvec',
            'hhess:V:hess',
            'tan1:V:hevec2',
            'strn:V:heval2:0:-1',
            'lthr:V:heval2:-70:-1',
            'sthr:V:heval2:-70:-1']

pullDemo.run(npos,
             init=[teem.pullInitMethodRandom, 300],
             efs=False,
             vol=vol,
             info=infoList,
             energy={'type':teem.pullInterTypeJustR, 'r':'cwell:0.6,-0.002'},
             iterMax=200,
             radSpace=0.05)
           
pullDemo.volFree(vol)

if (teem.nrrdSave("npos.nrrd", npos, None)):
    estr = teem.biffGetDone('nrrd')
    print "problem running system:\n%s" % estr
    sys.exit(1)
