namespace WireCell {

    /** Spread deposits on a plane.
     *
     * Maintain a buffer in time (longitudinal to drift direction) and
     * space (transverse to drift direction, parallel to wire pitch
     * direction) of energy depositions.
     *
     * The change is spread in both directions according to given
     * sigma and according to the longitudinal drift distance that the
     * deposition has or must travel (prior deposition is checked of
     * originating point).
     */
    class DepoPlane {
        Ray m_pitch;
        double m_now, m_tick, m_sigmaT, m_sigmaL, m_drift_velocity;

       public:
        DepoPlane(Ray wire_pitch,  // point from center of wire zero to wire one along pitch direction
                  double start_time, double tick, double sigmaT, double sigmaL, double drift_velocity);

        /// Add a deposition to the buffer.  Return true if we have enough to pop.
        bool add(IDepo::pointer depo);

        /// Return next tick worth of charge.  Vector is indexed by
        /// wire index starting from wire zero as determined by pitch
        /// ray.  An empty vector indicates underflow and more
        /// depositions should be add()'ed.
        std::vector<double> pop();
    };

}  // namespace WireCell
