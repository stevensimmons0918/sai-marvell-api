#!/usr/bin/python
# coding: utf-8
#
import matplotlib.pyplot as plt
import json
import numpy as np

#choose the path of the .txt file which contains the data from the test
path = 'C:/Users/harelh/Desktop/PID tests/PID_test21.txt'
with open(path) as json_file:
    data = json.load(json_file)

""" this function marks on the plot the pick temperature,
    and if exists the first time the temperature hit below the set point after the second the pick of the
    selected part of the test.
    Args:
        y       (int[])         : array of data
        ax      (numpy.ndarray) : x axis of the plot
        sp      (int)           : set point of PID
        start   (int)           : start index of the selected part of the test
        stop    (int)           : stop index of the selected part of the test
        xLocationOnGrap (int)   : x location on the graph of the points

    Note:
        start and stop paramaters should be the same as in the test code.
        for exmaple: if part 2 of the test is between 15:45 min then start = 90, stop = 270.
"""
def annot_points(y, ax, sp, start, stop, xLocationOnGrap):
    x1 = range(0, len(y), 1)
    xmax = start + x1[np.argmax(y[start:stop])] # x index of maximum value in y array
    ymax = max(y[start:stop])                   # maximum value in y array

    # point text, location and annotate
    text = "x={:.1f}, y={:.1f}".format(xmax * 10, ymax)
    if not ax:
        ax = plt.gca()
    bbox_props = dict(boxstyle="square,pad=0.3", fc="w", ec="k", lw=0.72)
    arrowprops = dict(arrowstyle="->")
    kw = dict(xycoords='data', textcoords="axes fraction",
              arrowprops=arrowprops, bbox=bbox_props, ha="right", va="top")
    # ax.annotate(text, xy=(xmax, ymax), xytext=(0.94,0.96), **kw)
    ax.annotate(text, xy=(xmax, ymax), xytext=(xLocationOnGrap, 0.2), **kw)

    # annotate the second point
    firstYVal = 0
    firstYIdx = 0

    # search for the first point below the SP
    for i in range(start, stop):
        if y[i] <= sp and y[i - 1] > sp:
            firstYVal = y[i]
            firstYIdx = i
            break

    # point text, location and annotate
    text2 = "x={:.1f}, y={:.1f}".format(firstYIdx * 10, firstYVal)
    bbox_props = dict(boxstyle="square,pad=0.3", fc="w", ec="k", lw=0.72)
    arrowprops = dict(arrowstyle="->", connectionstyle="angle,angleA=0,angleB=90")
    kw = dict(xycoords='data', textcoords="axes fraction",
              arrowprops=arrowprops, bbox=bbox_props, ha="right", va="center")
    ax.annotate(text2, xy=(firstYIdx, firstYVal), xytext=(xLocationOnGrap, 0.1), **kw)


for PID in data['PID list']:
    fig, axs = plt.subplots(2, 3,figsize=(16, 9),dpi=300)  # type: (object, object)
    PidName = PID['name']
    fig.suptitle(PidName, fontsize=16)

    # eagle
    axs[0, 0].plot(PID['sensors'][0]['temperature'])
    axs[0, 0].set_title(PID['sensors'][0]['name'])
    annot_points(PID['sensors'][0]['temperature'], axs[0, 0], 80, 90, 270, 0.94)
    annot_points(PID['sensors'][0]['temperature'], axs[0, 0], 80, 0, 90, 0.5)

    # CPU
    axs[0, 1].plot(PID['sensors'][1]['temperature'], 'tab:orange')
    axs[0, 1].set_title(PID['sensors'][1]['name'])
    annot_points(PID['sensors'][1]['temperature'], axs[0, 1], 80, 90, 270, 0.94)
    annot_points(PID['sensors'][1]['temperature'], axs[0, 1], 80, 0, 90, 0.5)


    # raven
    axs[1, 0].plot(PID['sensors'][2]['temperature'], 'tab:green')
    axs[1, 0].set_title(PID['sensors'][2]['name'])
    annot_points(PID['sensors'][2]['temperature'], axs[1, 0], 80, 90, 270,0.94)
    annot_points(PID['sensors'][2]['temperature'], axs[1, 0], 80, 0, 90, 0.5)

    # board
    axs[1, 1].plot(PID['sensors'][3]['temperature'], 'tab:red')
    axs[1, 1].set_title(PID['sensors'][3]['name'])
    annot_points(PID['sensors'][3]['temperature'], axs[1, 1], 70, 90, 270, 0.94)
    annot_points(PID['sensors'][3]['temperature'], axs[1, 1], 70, 0, 90, 0.5)

    # transceivers
    axs[0, 2].plot(PID['sensors'][4]['temperature'], 'tab:blue')
    axs[0, 2].set_title(PID['sensors'][4]['name'])
    annot_points(PID['sensors'][4]['temperature'], axs[0, 2], 60, 90, 270, 0.94)
    annot_points(PID['sensors'][4]['temperature'], axs[0, 2], 60, 0, 90, 0.5)

    # Fan Speed
    axs[1, 2].plot(PID['sensors'][5]['speed'], 'tab:orange')
    axs[1, 2].set_title(PID['sensors'][5]['name'])

    plt.savefig('C:/Users/harelh/Desktop/PID tests/' + PidName + '.png')

