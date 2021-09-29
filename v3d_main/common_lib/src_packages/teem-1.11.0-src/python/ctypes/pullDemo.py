##
##  pullDemo.py: sample wrappers for pull API
##  Copyright (C) 2010, 2009, Gordon Kindlmann
##
##  Permission is hereby granted, free of charge, to any person obtaining
##  a copy of this software and associated documentation files (the
##  "Software"), to deal in the Software without restriction, including
##  without limitation the rights to use, copy, modify, merge, publish,
##  distribute, sublicense, and/or sell copies of the Software, and to
##  permit persons to whom the Software is furnished to do so, subject to
##  the following conditions:
##
##  The above copyright notice and this permission notice shall be
##  included in all copies or substantial portions of the Software.
##
##  THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND,
##  EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF
##  MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND
##  NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS BE
##  LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION
##  OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION
##  WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.
##

'''
This is really hastily written, and doesn't conform in the least
to good conventions of Python coding. The worst part is using exit()
instead of raising exceptions when tings go wrong. 

The "API" here (this python interface to the pull library) is not well
thought-out, and won't be helpful for integrating particles in other
applications.  Better designed interfaces should come in the future,
as more and more of Teem develops idiomatic python interfaces.

The code here is mainly to serve as a demo of using the pull library
in Teem (whose API should be quite stable), and to simplify
documenting using pull to produce typical results.
'''

import teem
import ctypes
import sys

##
## volLoad: loads in volumes
##
def volLoad(vlist, cachePath, kssBlurStr, verbose):
    kSSblur = teem.nrrdKernelSpecNew()
    if (teem.nrrdKernelSpecParse(kSSblur, kssBlurStr)):
        estr = teem.biffGetDone('nrrd')
        print "problem with kernel:\n%s" % estr
        sys.exit(1)
    vol = (ctypes.POINTER(teem.meetPullVol) * len(vlist))()
    E = 0
    for i in range(len(vlist)):
        vol[i] = teem.meetPullVolNew()
        if (not E):
            E += teem.meetPullVolParse(vol[i], vlist[i])
    if (not E): 
        E += teem.meetPullVolLoadMulti(vol, len(vol),
                                       cachePath, kSSblur,
                                       teem.nrrdBoundaryWrap, 0.0,
                                       verbose)
    if (E):
        estr = teem.biffGetDone('meet')
        print "problem with volumes:\n%s" % estr
        sys.exit(1)
    teem.nrrdKernelSpecNix(kSSblur)
    return vol

def volFree(vol):
    for i in range(len(vol)):
        vol[i] = teem.meetPullVolNix(vol[i])

##
## especParse: returns a length-4 list of the arguments to pass 
## to pullInterEnergySet. Arguments are:
## type: from pullIterType* enum
## r: string definition for energy function along space (especR)
## s: string definition for energy function along scale (especS)
## w: string definition for windowing energy function (especW)
##
def energyParse(**kwargs):
    type = kwargs['type']
    if (teem.pullInterTypeJustR == type
        or teem.pullInterTypeUnivariate == type):
        espec = [teem.pullEnergySpecNew(), None, None]
    elif (teem.pullInterTypeSeparable == type):
        espec = [teem.pullEnergySpecNew(),
                 teem.pullEnergySpecNew(), None]
    elif (teem.pullInterTypeAdditive == type):
        espec = [teem.pullEnergySpecNew(),
                 teem.pullEnergySpecNew(),
                 teem.pullEnergySpecNew()]
    lett = ['r', 's', 'w']
    E = 0
    for i in range(3):
        if (not E and espec[i]):
            E += teem.pullEnergySpecParse(espec[i], kwargs[lett[i]])
    if (E):
        estr = teem.biffGetDone('pull')
        print "problem with infos:\n%s" % estr
        sys.exit(1)
    ret = [type, espec[0], espec[1], espec[2]]
    return ret

def energyFree(espec):
    espec[1] = teem.pullEnergySpecNix(espec[1])
    espec[2] = teem.pullEnergySpecNix(espec[2])
    espec[3] = teem.pullEnergySpecNix(espec[3])
    return

##
## initSet: calls (and returns value from) the appropriate 
## pullInitMethod* function. Possibilities are:
## [teem.pullInitMethodRandom, num]
## [teem.pullInitMethodPointPerVoxel, 
##     ppv, zslcmin, zslcmax, alongscalenum, jitter]
## [teem.pullInitMethodGivenPos, npos]
##
def initSet(pctx, info):
    if (teem.pullInitMethodRandom == info[0]):
        return teem.pullInitRandomSet(pctx, info[1])
    elif (teem.pullInitMethodPointPerVoxel == info[0]):
        return teem.pullInitPointPerVoxelSet(pctx, info[1],
                                             info[2], info[3],
                                             info[4], info[5])
    elif (teem.pullInitMethodGivenPos == info[0]):
        return teem.pullInitGivenPosSet(pctx, info[1])


## place for storing pullContext->sysParm.gamma
gamma = 0

## way of getting arguments that allows for checking whether
## there were any bogus options
def a(args, a, default=None):
    if default is None:
        ret = args.get(a)
    else:
        ret = args.get(a, default)
    if a in args:
        del args[a]
    return ret

##
## All the set-up and commands that are required to do a pullContext run
##
def run(nposOut, **args):
    global gamma
    if (not ('vol' in args and 'info' in args and 'efs' in args)):
        print "run: didn't get vol, info, and efs args"
        sys.exit(1)
        
    # learn args, with defaults
    vol = a(args, 'vol')
    infoStr = a(args, 'info')
    efs = a(args, 'efs')
    init = a(args, 'init', [teem.pullInitMethodRandom, 100])
    energyDict = a(args, 'energy', {'type':teem.pullInterTypeJustR, 
                                    'r':'qwell:0.68'})
    verbose = a(args, 'verbose', 1)
    rngSeed = a(args, 'rngSeed', 42)
    nobin = a(args, 'nobin', False)
    noadd = a(args, 'noadd', False)
    ac3c = a(args, 'ac3c', False)
    usa = a(args, 'usa', False)
    nave = a(args, 'nave', True)
    cbst = a(args, 'cbst', True)
    ratb = a(args, 'ratb', True)
    lti = a(args, 'lti', False)
    npcwza = a(args, 'npcwza', False)
    ubfgl = a(args, 'ubfgl', False)
    iterMin = a(args, 'iterMin', 0)
    iterMax = a(args, 'iterMax', 100)
    snap = a(args, 'snap', 0)
    pcp = a(args, 'pcp', 5)
    iad = a(args, 'iad', 10)
    radSpace = a(args, 'radSpace', 1)
    radScale = a(args, 'radScale', 1)
    alpha = a(args, 'alpha', 0.5)
    beta = a(args, 'beta', 0.5)
    gamma = a(args, 'gamma', 1)
    stepInitial = a(args, 'step', 1)
    ssBack = a(args, 'ssBack', 0.2)
    ssOppor = a(args, 'ssOppor', 1.1)
    pbm = a(args, 'pbm', 50)
    k00Str = a(args, 'k00', 'c4h')
    k11Str = a(args, 'k11', 'c4hd')
    k22Str = a(args, 'k22', 'c4hdd')
    kSSreconStr = a(args, 'kSSrecon', 'hermite')
    eip = a(args, 'eip', 0.00005)
    eiphl = a(args, 'eiphl', 0)
    fnnm = a(args, 'fnnm', 0.25)
    edmin = a(args, 'edmin', 0.00001)
    edpcmin = a(args, 'edpcmin', 0.01)
    maxci = a(args, 'maxci', 15)
    if args:
        print
        print "sorry, got unrecognized arguments:"
        print args
        sys.exit(1)

    # create pullContext and set up all its state.  Its kind of silly that
    # the pullContext is created anew everytime we want to run it, but thats
    # because currently the pullContext doesn't have the kind of internal
    # network of flags (like the gageContext does) that allow the context
    # to be modifyied and re-used indefinitely.  This will be fixed after
    # the Teem v1.11 release.
    pctx = teem.pullContextNew()
    energyList = energyParse(**energyDict)
    if (teem.pullVerboseSet(pctx, verbose) or
        teem.pullRngSeedSet(pctx, rngSeed) or
        teem.pullFlagSet(pctx, teem.pullFlagNixAtVolumeEdgeSpace, nave) or
        teem.pullFlagSet(pctx, teem.pullFlagConstraintBeforeSeedThresh, 
                         cbst) or
        teem.pullFlagSet(pctx, teem.pullFlagEnergyFromStrength, efs) or
        teem.pullFlagSet(pctx, teem.pullFlagRestrictiveAddToBins, ratb) or
        teem.pullFlagSet(pctx, teem.pullFlagNoPopCntlWithZeroAlpha, npcwza) or
        teem.pullFlagSet(pctx, teem.pullFlagUseBetaForGammaLearn, ubfgl) or
        teem.pullFlagSet(pctx, teem.pullFlagBinSingle, nobin) or
        teem.pullFlagSet(pctx, teem.pullFlagNoAdd, noadd) or
        teem.pullFlagSet(pctx,
                         teem.pullFlagAllowCodimension3Constraints,
                         ac3c) or
        teem.pullInitUnequalShapesAllowSet(pctx, usa) or
        teem.pullIterParmSet(pctx, teem.pullIterParmMin, iterMin) or
        teem.pullIterParmSet(pctx, teem.pullIterParmMax, iterMax) or
        teem.pullIterParmSet(pctx, teem.pullIterParmSnap, snap) or
        teem.pullIterParmSet(pctx, teem.pullIterParmConstraintMax, maxci) or
        teem.pullIterParmSet(pctx, teem.pullIterParmPopCntlPeriod, pcp) or
        teem.pullIterParmSet(pctx, teem.pullIterParmAddDescent, iad) or
        teem.pullIterParmSet(pctx,
                             teem.pullIterParmEnergyIncreasePermitHalfLife,
                             eiphl) or
        teem.pullSysParmSet(pctx, teem.pullSysParmStepInitial, stepInitial) or
        teem.pullSysParmSet(pctx, teem.pullSysParmRadiusSpace, radSpace) or
        teem.pullSysParmSet(pctx, teem.pullSysParmRadiusScale, radScale) or
        teem.pullSysParmSet(pctx, teem.pullSysParmAlpha, alpha) or
        teem.pullSysParmSet(pctx, teem.pullSysParmBeta, beta) or
        teem.pullSysParmSet(pctx, teem.pullSysParmGamma, gamma) or
        teem.pullSysParmSet(pctx, teem.pullSysParmEnergyIncreasePermit, eip) or
        teem.pullSysParmSet(pctx, teem.pullSysParmFracNeighNixedMax, fnnm) or
        teem.pullSysParmSet(pctx, teem.pullSysParmEnergyDecreaseMin, edmin) or
        teem.pullSysParmSet(pctx, teem.pullSysParmEnergyDecreasePopCntlMin,
                            edpcmin) or
        teem.pullSysParmSet(pctx, teem.pullSysParmBackStepScale, ssBack) or
        teem.pullSysParmSet(pctx, teem.pullSysParmOpporStepScale, ssOppor) or
        teem.pullProgressBinModSet(pctx, pbm) or
        teem.pullInterEnergySet(pctx, energyList[0], energyList[1],
                                energyList[2], energyList[3])):
        estr = teem.biffGetDone('pull')
        print "problem with set-up:\n%s" % estr
        sys.exit(1)

    # The meet library offers a slightly higher-level abstraction of
    # the pullInfo; this code parses a bunch of info specifications
    info = (ctypes.POINTER(teem.meetPullInfo) * len(infoStr))()
    for i in range(len(infoStr)):
        info[i] = teem.meetPullInfoNew()
        if (teem.meetPullInfoParse(info[i], infoStr[i])):
            estr = teem.biffGetDone('meet')
            print "problem with infos:\n%s" % estr
            sys.exit(1)

    # Create all the kernels needed for reconstruction
    k00 = teem.nrrdKernelSpecNew()
    k11 = teem.nrrdKernelSpecNew()
    k22 = teem.nrrdKernelSpecNew()
    kSSrecon = teem.nrrdKernelSpecNew()
    if (teem.nrrdKernelSpecParse(k00, k00Str) or
        teem.nrrdKernelSpecParse(k11, k11Str) or
        teem.nrrdKernelSpecParse(k22, k22Str) or
        teem.nrrdKernelSpecParse(kSSrecon, kSSreconStr)):
        estr = teem.biffGetDone('nrrd')
        print "problem with kernels:\n%s" % estr
        sys.exit(1)

    # We do assume that the volumes came in loaded (as opposed to reloading
    # them here for every one); here we add all the volumes and infos into
    # the pullContext
    if (teem.meetPullVolAddMulti(pctx, vol, len(vol),
                                 k00, k11, k22, kSSrecon) or
        teem.meetPullInfoAddMulti(pctx, info, len(info))):
            estr = teem.biffGetDone('meet')
            print "problem with infos:\n%s" % estr
            sys.exit(1)

    # Final setup and the actual "pullRun" call
    if (initSet(pctx, init) or
        teem.pullInitLiveThreshUseSet(pctx, lti) or
        teem.pullStart(pctx) or
        teem.pullRun(pctx) or
        teem.pullOutputGet(nposOut, None, None, None, 0, pctx)):
        estr = teem.biffGetDone('pull')
        print "problem running system:\n%s" % estr
        sys.exit(1)

    # until there's a clean way to re-use a pullContext for different runs,
    # there's no good way for the user to call pullGammaLearn (because pctx
    # doesn't survive outside the call to "run".  So we call pullGammaLearn
    # it whenever it makes sense to do so and save the result in the global
    # "gamma"
    if (teem.pullInterTypeAdditive == pctx.contents.interType and
        teem.pullEnergyButterworthParabola.contents.name
        == pctx.contents.energySpecS.contents.energy.contents.name):
        if (teem.pullGammaLearn(pctx)):
            estr = teem.biffGetDone('pull')
            print "problem learning gamma:\n%s" % estr
            sys.exit(1)
        gamma = pctx.contents.sysParm.gamma
        print "pullGammaLearn returned", gamma

    teem.nrrdKernelSpecNix(k00)
    teem.nrrdKernelSpecNix(k11)
    teem.nrrdKernelSpecNix(k22)
    teem.nrrdKernelSpecNix(kSSrecon)
    energyFree(energyList)
    teem.pullContextNix(pctx)
    return
