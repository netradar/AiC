package com.vanxum.Aic;

interface dataReportInterface{

    void videoDataReport(byte[] data,int len);
    void audioDataReport(byte[] data,int len);
    void reportConnected();
}
interface examReportInterface{
    void reportExamFeedback();
}